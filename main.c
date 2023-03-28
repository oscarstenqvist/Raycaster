#include <SDL2/SDL.h>
#include <math.h>
#include <stdio.h>
#define PI 3.1415926536
#define WIDTH 1600
#define HEIGHT 800
#define MAPX 8
#define MAPY 8
#define DELAY 6
#define RAYS 1
#define FOV PI / 2
struct Mvsqr {
	float x, y, deltaX, deltaY, angle, width;
} typedef mvsqr;
void drawRay3D(SDL_Renderer *renderer, int i, int len) {
	SDL_Rect rect;
	rect.x = (WIDTH / 2) + i * ((WIDTH / 2) / RAYS); // Draw upper half
	rect.y = HEIGHT / 2;
	rect.w = (WIDTH / 2) / RAYS;
	rect.h = (MAPX * MAPY * (HEIGHT / 2)) / len;
	if (rect.h > HEIGHT / 2) {
		rect.h = HEIGHT / 2;
	}
	SDL_RenderFillRect(renderer, &rect);
	rect.h = -rect.h; // Draw lower half
	SDL_RenderFillRect(renderer, &rect);
}
void checkAngleLimits(float *angle) {
	if (*angle < 0) {
		*angle += 2 * PI;
	}
	if (*angle > 2 * PI) {
		*angle -= 2 * PI;
	}
}
void horizontalRay(mvsqr player, float rayAngle, int map[], float *rayHX, float *rayHY, float *hLen) {
	int dof = 1;
	int mapLineSpacing = HEIGHT / MAPY;
	int playerLine = player.y / mapLineSpacing;
	int nextMapLine = playerLine;
	int rayMapPosX, rayMapPosY, rayMapPos;
	while (dof < 8) {
		if (rayAngle > PI) { // Looking up
			if (dof != 1) {
				nextMapLine--;
			}
			*rayHY = nextMapLine * mapLineSpacing - 0.001; // ray is "inside" the tile above
			*rayHX = player.x - (player.y - *rayHY) / tan(rayAngle);
			*hLen = fabs((player.y - *rayHY) / sin(rayAngle));
		}
		if (rayAngle == PI || rayAngle == 0) { // Looking straight left or right
			*hLen = 9001;
			break;
		}
		if (rayAngle > 0 && rayAngle < PI) { // Looking down
			nextMapLine++;
			*rayHY = nextMapLine * mapLineSpacing;
			*rayHX = player.x - (player.y - *rayHY) / tan(rayAngle);
			*hLen = fabs((*rayHY - player.y) / sin(rayAngle));
		}
		rayMapPosX = *rayHX / mapLineSpacing;
		rayMapPosY = *rayHY / mapLineSpacing;
		rayMapPos = rayMapPosY * MAPX + rayMapPosX;
		if (rayMapPos >= MAPX * MAPY - 1 || rayMapPos < 0) {
			break;
		}
		if (map[rayMapPos] == 1) {
			break;
		}
		dof++;
	}
}
void verticalRay(mvsqr player, float rayAngle, int map[], float *rayVX, float *rayVY, float *vLen) {
	int dof = 1;
	int mapLineSpacing = (WIDTH / 2) / MAPX;
	int playerLine = player.x / mapLineSpacing;
	int nextMapLine = playerLine;
	int rayMapPosX, rayMapPosY, rayMapPos;
	while (dof < 8) {
		if (rayAngle > 3 * PI / 2 || rayAngle < PI / 2) { // Looking right
			nextMapLine++;
			*rayVX = nextMapLine * mapLineSpacing;
			*rayVY = tan(rayAngle) * (*rayVX - player.x) + player.y;
			*vLen = fabs((*rayVX - player.x) / cos(rayAngle));
		}
		if (rayAngle == PI / 2 || rayAngle == 3 * PI / 2) { // Looking straight up or down
			*vLen = 9001;
			break;
		}
		if (rayAngle > PI / 2 && rayAngle < 3 * PI / 2) { // Looking left
			if (dof != 1) {
				nextMapLine--;
			}
			*rayVX = nextMapLine * mapLineSpacing - 0.001; // ray is "inside" the left tile
			*rayVY = tan(rayAngle) * (*rayVX - player.x) + player.y;
			*vLen = fabs((player.x - *rayVX) / cos(rayAngle));
		}
		rayMapPosX = *rayVX / mapLineSpacing;
		rayMapPosY = *rayVY / mapLineSpacing;
		rayMapPos = rayMapPosY * MAPX + rayMapPosX;
		if (rayMapPos >= MAPX * MAPY - 1 || rayMapPos < 0) {
			break;
		}
		if (map[rayMapPos] == 1) {
			break;
		}
		dof++;
	}
}
void drawRays(SDL_Renderer *renderer, mvsqr player, int map[]) {
	float rayAngle = player.angle - FOV / 2.0;
	float rayVX, rayVY, vLen, rayHX, rayHY, hLen, shortLen;
	for (int i = 0; i < RAYS; i++) {
		checkAngleLimits(&rayAngle);
		verticalRay(player, rayAngle, map, &rayVX, &rayVY, &vLen);
		horizontalRay(player, rayAngle, map, &rayHX, &rayHY, &hLen);
		SDL_SetRenderDrawColor(renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
		if (vLen < hLen) {
			SDL_RenderDrawLine(renderer, player.x, player.y, rayVX, rayVY);
			shortLen = vLen;
			SDL_SetRenderDrawColor(renderer, 150, 0, 150, SDL_ALPHA_OPAQUE); // Shade vertical walls
		} else {
			SDL_RenderDrawLine(renderer, player.x, player.y, rayHX, rayHY);
			shortLen = hLen;
			SDL_SetRenderDrawColor(renderer, 255, 0, 255, SDL_ALPHA_OPAQUE);
		}

		// Fix fisheye effect
		float playerRayAngle = player.angle - rayAngle;
		checkAngleLimits(&playerRayAngle);
		shortLen = shortLen * cos(playerRayAngle); // Math magic
		drawRay3D(renderer, i, shortLen);
		rayAngle += FOV / (RAYS - 1);
	}
}
void drawPlayer(SDL_Renderer *renderer, mvsqr player) {
	SDL_Rect playerRect;
	playerRect.x = player.x - player.width / 2;
	playerRect.y = player.y - player.width / 2;
	playerRect.w = player.width;
	playerRect.h = player.width;
	SDL_SetRenderDrawColor(renderer, 0, 0, 255, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(renderer, &playerRect);
}
void draw3DBackground(SDL_Renderer *renderer) {
	SDL_Rect rect;
	rect.x = WIDTH / 2;
	rect.y = 0;
	rect.w = WIDTH / 2;
	rect.h = HEIGHT / 2;
	SDL_SetRenderDrawColor(renderer, 0, 255, 255, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(renderer, &rect);
	rect.y = HEIGHT / 2;
	rect.h = HEIGHT;
	SDL_SetRenderDrawColor(renderer, 0, 51, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(renderer, &rect);
}
void draw2DBackground(SDL_Renderer *renderer, int map[]) {
	SDL_Rect rect;
	for (int y = 0; y < MAPY; y++) {
		for (int x = 0; x < MAPX; x++) {
			rect.x = x * ((WIDTH / 2) / MAPX);
			rect.y = y * (HEIGHT / MAPY);
			rect.w = ((WIDTH / 2) / MAPX) - 2;
			rect.h = (HEIGHT / MAPY) - 2;
			if (map[y * MAPX + x] == 1) {
				SDL_SetRenderDrawColor(renderer, 255, 0, 255, SDL_ALPHA_OPAQUE);
			} else if (map[y * MAPX + x] == 0) {
				SDL_SetRenderDrawColor(renderer, 0, 51, 0, SDL_ALPHA_OPAQUE);
			}
			SDL_RenderFillRect(renderer, &rect);
		}
	}
}
int playerInWall(int playerX, int playerY, int map[]) {
	return map[playerY / (HEIGHT / MAPY) * MAPX + playerX / ((WIDTH / 2) / MAPX)];
}
int handleInputs(mvsqr *player, int map[]) {
	SDL_Event event;
	const Uint8 *currentKeyStates = SDL_GetKeyboardState(NULL);
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT) {
			return 0;
		}
	}
	if (currentKeyStates[SDL_SCANCODE_W]) {
		player->x += player->deltaX;
		player->y += player->deltaY;
		if (player->x > WIDTH / 2 || player->x < 0 ||
			playerInWall(player->x, player->y, map)) {
			player->x -= player->deltaX;
			player->y -= player->deltaY;
		}
	}
	if (currentKeyStates[SDL_SCANCODE_S]) {
		player->x -= player->deltaX;
		player->y -= player->deltaY;
		if (player->x > WIDTH / 2 || player->x < 0 ||
			playerInWall(player->x, player->y, map)) {
			player->x += player->deltaX;
			player->y += player->deltaY;
		}
	}
	if (currentKeyStates[SDL_SCANCODE_A]) {
		player->angle -= 0.03;
		checkAngleLimits(&player->angle);
		player->deltaX = cos(player->angle) * 2.5;
		player->deltaY = sin(player->angle) * 2.5;
	}
	if (currentKeyStates[SDL_SCANCODE_D]) {
		player->angle += 0.03;
		checkAngleLimits(&player->angle);
		player->deltaX = cos(player->angle) * 2.5;
		player->deltaY = sin(player->angle) * 2.5;
	}
	if (currentKeyStates[SDL_SCANCODE_ESCAPE]) {
		return 0;
	}
	return 1;
}
int isMouseEvent(void *userdata, SDL_Event *event) { // Remove mouse movement from event handler
	return event->type != SDL_MOUSEMOTION;
}
int main(int argc, char *argv[]) {
	int map[MAPY * MAPX] = {
		1, 1, 1, 1, 1, 1, 1, 1,
		1, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 1, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 1, 0, 1,
		1, 0, 0, 0, 0, 1, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 1,
		1, 1, 1, 1, 1, 1, 1, 1};
	SDL_Window *window = NULL;
	SDL_Renderer *renderer = NULL;
	SDL_Init(SDL_INIT_VIDEO);
	SDL_CreateWindowAndRenderer(WIDTH, HEIGHT, 0, &window, &renderer);
	SDL_SetEventFilter(isMouseEvent, NULL);
	mvsqr player = {10 + (WIDTH / 2) / MAPX, 10 + HEIGHT * 4 / MAPY, cos(0) * 2.5, sin(0) * 2.5, 0, 20};
	while (handleInputs(&player, map)) {
		SDL_SetRenderDrawColor(renderer, 127, 127, 127, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(renderer);
		draw2DBackground(renderer, map);
		draw3DBackground(renderer);
		drawPlayer(renderer, player);
		drawRays(renderer, player, map);
		SDL_RenderPresent(renderer);
		SDL_Delay(DELAY);
	}
	SDL_DestroyWindow(window);
	SDL_Quit();
}