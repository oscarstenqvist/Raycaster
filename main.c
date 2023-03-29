#include <SDL2/SDL.h>
#include <math.h>
#include <stdio.h>
#define PI 3.1415926536
#define MAPX 10
#define MAPY 10
#define MAPLINE 100
#define WIDTH 2 * MAPX *MAPLINE
#define HEIGHT MAPY *MAPLINE
#define DELAY 6
#define RAYS 100
#define FOV PI / 2
#define DOF MAPX *MAPY
struct Mvsqr {
	float x, y, deltaX, deltaY, angle, width;
} typedef mvsqr;
void checkAngleLimits(float *angle) {
	if (*angle < 0)
		*angle += 2 * PI;
	if (*angle > 2 * PI)
		*angle -= 2 * PI;
}
void setColor(SDL_Renderer *renderer, int tileColor, float shade) {
	switch (tileColor) {
	case 0:
		SDL_SetRenderDrawColor(renderer, 0 * shade, 0 * shade, 0 * shade, SDL_ALPHA_OPAQUE);
		break;
	case 1:
		SDL_SetRenderDrawColor(renderer, 255 * shade, 255 * shade, 255 * shade, SDL_ALPHA_OPAQUE);
		break;
	case 2:
		SDL_SetRenderDrawColor(renderer, 0 * shade, 0 * shade, 255 * shade, SDL_ALPHA_OPAQUE);
		break;
	case 3:
		SDL_SetRenderDrawColor(renderer, 0 * shade, 255 * shade, 0 * shade, SDL_ALPHA_OPAQUE);
		break;
	case 4:
		SDL_SetRenderDrawColor(renderer, 255 * shade, 0 * shade, 255 * shade, SDL_ALPHA_OPAQUE);
		break;
	case 5:
		SDL_SetRenderDrawColor(renderer, 255 * shade, 127 * shade, 0 * shade, SDL_ALPHA_OPAQUE);
		break;
	case 6:
		SDL_SetRenderDrawColor(renderer, 255 * shade, 0 * shade, 0 * shade, SDL_ALPHA_OPAQUE);
		break;
	case 7:
		SDL_SetRenderDrawColor(renderer, 255 * shade, 255 * shade, 0 * shade, SDL_ALPHA_OPAQUE);
		break;
	}
}
void drawRay3D(SDL_Renderer *renderer, int i, int len) {
	SDL_Rect rect;
	rect.x = (WIDTH / 2) + i * ((WIDTH / 2) / RAYS); // Draw upper half
	rect.y = HEIGHT / 2;
	rect.w = (WIDTH / 2) / RAYS;
	rect.h = 0.5 * (MAPX * MAPY * (HEIGHT / 2)) / len;
	if (rect.h > HEIGHT / 2) {
		rect.h = HEIGHT / 2;
	}
	SDL_RenderFillRect(renderer, &rect);
	rect.h = -rect.h; // Draw lower half
	SDL_RenderFillRect(renderer, &rect);
}
void horizontalRay(mvsqr player, float rayAngle, int map[], float *rayHX, float *rayHY, float *hLen, int *hColor) {
	int dof = 1;
	int mapLineSpacing = HEIGHT / MAPY;
	int playerLine = player.y / mapLineSpacing;
	int nextMapLine = playerLine + 1;
	int rayMapPosX, rayMapPosY, rayMapPos;
	while (dof < DOF) {
		if (rayAngle > PI) { // Facing up
			nextMapLine--;
			*rayHY = nextMapLine * mapLineSpacing;
			rayMapPosY = nextMapLine - 1;
			*hLen = fabs((player.y - *rayHY) / sin(rayAngle));
		}
		if (rayAngle > 0 && rayAngle < PI) { // Facing down
			*rayHY = nextMapLine * mapLineSpacing;
			rayMapPosY = nextMapLine;
			*hLen = fabs((*rayHY - player.y) / sin(rayAngle));
			nextMapLine++;
		}
		if (rayAngle == PI || rayAngle == 0) { // Facing straight left or right
			*hLen = 9001;
			break;
		}
		*rayHX = player.x - (player.y - *rayHY) / tan(rayAngle);
		rayMapPosX = *rayHX / mapLineSpacing;
		rayMapPos = rayMapPosY * MAPX + rayMapPosX;
		if (rayMapPos >= MAPX * MAPY - 1 || rayMapPos < 0) {
			break;
		}
		if (map[rayMapPos] != 0) {
			*hColor = map[rayMapPos];
			break;
		}
		dof++;
	}
}
void verticalRay(mvsqr player, float rayAngle, int map[], float *rayVX, float *rayVY, float *vLen, int *vColor) {
	int dof = 1;
	int mapLineSpacing = (WIDTH / 2) / MAPX;
	int playerLine = player.x / mapLineSpacing;
	int nextMapLine = playerLine + 1;
	int rayMapPosX, rayMapPosY, rayMapPos;
	while (dof < DOF) {
		if (rayAngle > PI / 2 && rayAngle < 3 * PI / 2) { // Facing left
			nextMapLine--;
			*rayVX = nextMapLine * mapLineSpacing;
			rayMapPosX = nextMapLine - 1;
			*vLen = fabs((player.x - *rayVX) / cos(rayAngle));
		}
		if (rayAngle > 3 * PI / 2 || rayAngle < PI / 2) { // Facing right
			*rayVX = nextMapLine * mapLineSpacing;
			rayMapPosX = nextMapLine;
			*vLen = fabs((*rayVX - player.x) / cos(rayAngle));
			nextMapLine++;
		}
		if (rayAngle == PI / 2 || rayAngle == 3 * PI / 2) { // Facing straight up or down
			*vLen = 9001;
			break;
		}
		*rayVY = tan(rayAngle) * (*rayVX - player.x) + player.y;
		rayMapPosY = *rayVY / mapLineSpacing;
		rayMapPos = rayMapPosY * MAPX + rayMapPosX;
		if (rayMapPos >= MAPX * MAPY - 1 || rayMapPos < 0) {
			break;
		}
		if (map[rayMapPos] != 0) {
			*vColor = map[rayMapPos];
			break;
		}
		dof++;
	}
}
void drawRays(SDL_Renderer *renderer, mvsqr player, int map[]) {
	float rayAngle = player.angle - FOV / 2.0;
	float rayVX, rayVY, vLen, rayHX, rayHY, hLen, shortLen, shade;
	int vColor, hColor, shortColor;
	for (int i = 0; i < RAYS; i++) {
		checkAngleLimits(&rayAngle);
		verticalRay(player, rayAngle, map, &rayVX, &rayVY, &vLen, &vColor);
		horizontalRay(player, rayAngle, map, &rayHX, &rayHY, &hLen, &hColor);
		SDL_SetRenderDrawColor(renderer, 255, 255, 0, SDL_ALPHA_OPAQUE);
		if (vLen < hLen) {
			SDL_RenderDrawLine(renderer, player.x, player.y, rayVX, rayVY);
			shortLen = vLen;
			shortColor = vColor;
			shade = 1;
		} else {
			SDL_RenderDrawLine(renderer, player.x, player.y, rayHX, rayHY);
			shortLen = hLen;
			shortColor = hColor;
			shade = 0.6;
		}
		setColor(renderer, shortColor, shade);

		float playerRayAngle = player.angle - rayAngle; // Fix fisheye effect
		checkAngleLimits(&playerRayAngle);
		shortLen = shortLen * cos(playerRayAngle); // Math magic
		drawRay3D(renderer, i, shortLen);
		rayAngle += FOV / (RAYS - 1);
	}
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
	SDL_SetRenderDrawColor(renderer, 129, 82, 0, SDL_ALPHA_OPAQUE);
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
			setColor(renderer, map[y * MAPX + x], 1);
			SDL_RenderFillRect(renderer, &rect);
		}
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
int playerInWall(int playerX, int playerY, int map[]) {
	return map[playerY / (HEIGHT / MAPY) * MAPX + playerX / ((WIDTH / 2) / MAPX)];
}
void movePlayer(mvsqr *player, int map[], int forward) {
	player->x += forward * player->deltaX;
	player->y += forward * player->deltaY;
	if (player->x > WIDTH / 2 || player->x < 0 || playerInWall(player->x, player->y, map)) {
		player->x -= forward * player->deltaX;
		player->y -= forward * player->deltaY;
	}
}
void rotatePlayer(mvsqr *player, int right) {
	player->angle += right * 0.02;
	checkAngleLimits(&player->angle);
	player->deltaX = cos(player->angle) * 2.5;
	player->deltaY = sin(player->angle) * 2.5;
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
		movePlayer(player, map, 1);
	}
	if (currentKeyStates[SDL_SCANCODE_S]) {
		movePlayer(player, map, -1);
	}
	if (currentKeyStates[SDL_SCANCODE_D]) {
		rotatePlayer(player, 1);
	}
	if (currentKeyStates[SDL_SCANCODE_A]) {
		rotatePlayer(player, -1);
	}
	if (currentKeyStates[SDL_SCANCODE_ESCAPE]) {
		return 0;
	}
	return 1;
}
int main(int argc, char *argv[]) {
	// 1->Pink, 2->Blue, 3->Green, 4->White
	int map[MAPY * MAPX] = {
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 6, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 5, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 2, 0, 0, 3, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 4, 0, 0, 7, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
	SDL_Window *window = NULL;
	SDL_Renderer *renderer = NULL;
	SDL_Init(SDL_INIT_VIDEO);
	SDL_CreateWindowAndRenderer(WIDTH, HEIGHT, 0, &window, &renderer);
	mvsqr player = {10 + (WIDTH / 2) / MAPX, 10 + HEIGHT * 4 / MAPY, cos(0.2) * 2.5, sin(0.2) * 2.5, 0.2, 20};
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