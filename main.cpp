#define _ITERATOR_DEBUG_LEVEL 0
#define _CRT_SECURE_NO_WARNINGS      
#include "raylib.h"
#include "raymath.h"
#include <vector>
#include <queue>
#include <map>
#include <algorithm>
#include <string>
#include <stack>
#include <random>
// Force the linker to find the correct debug symbols
#pragma comment(lib, "raylib.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "vcruntimed.lib")
#pragma comment(lib, "ucrtd.lib")
using namespace std;
const int MAZE_SIZE = 31;
int maze[MAZE_SIZE][MAZE_SIZE];
int levelCount = 1;
struct Point {
	int x, y;
	bool operator==(const Point& o) const { return x == o.x && y == o.y; }
	bool operator<(const Point& o) const { return tie(x, y) < tie(o.x, o.y); }
};
// --- Maze Generation (Recursive Backtracker) ---
void GenerateNewLevel() {
	for (int i = 0; i < MAZE_SIZE; i++)
		for (int j = 0; j < MAZE_SIZE; j++) maze[i][j] = 1;
	stack<Point> st;
	Point start = { 1, 1 };
	maze[start.x][start.y] = 0;
	st.push(start);

	int dx[] = { 0, 0, 2, -2 }, dy[] = { 2, -2, 0, 0 };
	while (!st.empty()) {
		Point curr = st.top();
		vector<int> dirs = { 0, 1, 2, 3 };
		shuffle(dirs.begin(), dirs.end(), mt19937(random_device()()));
		bool found = false;
		for (int d : dirs) {
			int nx = curr.x + dx[d], ny = curr.y + dy[d];
			if (nx > 0 && nx < MAZE_SIZE - 1 && ny > 0 && ny < MAZE_SIZE - 1 &&
				maze[nx][ny] == 1) {
				maze[nx][ny] = 0;
				maze[curr.x + dx[d] / 2][curr.y + dy[d] / 2] = 0;
				st.push({ nx, ny });
				found = true;
				break;
			}
		}
		if (!found) st.pop();
	}
}
// --- A* Solver ---
vector<Point> GetAIPath(Point start, Point end) {
	auto h = [&](Point p) { return abs(p.x - end.x) + abs(p.y - end.y); };
	priority_queue<pair<int, Point>, vector<pair<int, Point>>, greater<pair<int,
		Point>>> pq;
	map<Point, Point> parent;
	map<Point, int> g;
	pq.push({ h(start), start });
	g[start] = 0;
	int dx[] = { 1, -1, 0, 0 }, dy[] = { 0, 0, 1, -1 };
	while (!pq.empty()) {
		Point c = pq.top().second; pq.pop();
		if (c == end) {
			vector<Point> path;
			while (!(c == start)) { path.push_back(c); c = parent[c]; }
			reverse(path.begin(), path.end());
			return path;
		}
		for (int i = 0; i < 4; i++) {
			Point n = { c.x + dx[i], c.y + dy[i] };
			if (n.x >= 0 && n.x < MAZE_SIZE && n.y >= 0 && n.y < MAZE_SIZE &&
				maze[n.x][n.y] == 0) {
				if (g.find(n) == g.end() || g[c] + 1 < g[n]) {
					g[n] = g[c] + 1; parent[n] = c;
					pq.push({ g[n] + h(n), n });
				}
			}
		}
	}
	return {};
}
int main() {
	InitWindow(1280, 720, "3D Maze - Infinite Levels");
	// Initial Setup

	GenerateNewLevel();
	Point goal = { MAZE_SIZE - 2, MAZE_SIZE - 2 };
	vector<Point> path = GetAIPath({ 1, 1 }, goal);
	Camera3D camera = { {1.0f, 0.5f, 1.0f}, {2.0f, 0.5f, 2.0f}, {0.0f, 1.0f, 0.0f},
	65.0f, 0 };
	int pathIdx = 0;
	bool isTopDown = false;
	SetTargetFPS(60);
	while (!WindowShouldClose()) {
		if (IsKeyPressed(KEY_T)) isTopDown = !isTopDown;
		// --- LEVEL TRANSITION LOGIC ---
		if (pathIdx >= path.size()) {
			levelCount++;
			GenerateNewLevel();
			path = GetAIPath({ 1, 1 }, goal);
			pathIdx = 0;
			camera.position = { 1.0f, 0.5f, 1.0f };
		}
		// --- AUTO-PILOT & CAMERA ---
		if (!isTopDown && pathIdx < path.size()) {
			Vector3 target = { (float)path[pathIdx].x, 0.5f +
			(float)sin((float)GetTime() * 10.0f) * 0.03f, (float)path[pathIdx].y };
			camera.position = Vector3Lerp(camera.position, target, 0.1f);
			camera.target = Vector3Lerp(camera.target, target, 0.2f);
			if (Vector3Distance(camera.position, target) < 0.1f) pathIdx++;
			camera.up = { 0, 1, 0 };
		}
		else if (isTopDown) {
			camera.position = { MAZE_SIZE / 2.0f, 25.0f, MAZE_SIZE / 2.0f };
			camera.target = { MAZE_SIZE / 2.0f, 0.0f, MAZE_SIZE / 2.0f };
			camera.up = { 0, 0, -1 };
		}
		BeginDrawing();
		ClearBackground(BLACK);
		DrawRectangle(0, 0, 1280, 360, SKYBLUE);
		DrawRectangle(0, 360, 1280, 360, DARKGREEN);
		BeginMode3D(camera);
		for (int x = 0; x < MAZE_SIZE; x++) {
			for (int y = 0; y < MAZE_SIZE; y++) {
				if (maze[x][y] == 1) {
					DrawCube({ (float)x, 0.5f, (float)y }, 1, 1, 1, GRAY);
					DrawCubeWires({ (float)x, 0.5f, (float)y }, 1, 1, 1, BLACK);
				}
			}
		}
		DrawCubeWires({ (float)goal.x, 0.5f, (float)goal.y }, 0.6f, 0.6f, 0.6f,
			ColorFromHSV(GetTime() * 100, 1, 1));
		EndMode3D();
		// UI Overlay
		DrawRectangle(10, 10, 200, 70, Fade(BLACK, 0.5f));
		DrawText(TextFormat("LEVEL: %i", levelCount), 20, 20, 20, GOLD);
		DrawText("[T] TOGGLE VIEW", 20, 45, 15, WHITE);

		// Progress Bar
		float progress = (float)pathIdx / path.size();
		DrawRectangle(20, 700, 1240 * progress, 10, LIME);
		// --- MINIMAP & COMPASS HUD ---
		int tileSize = 5; // Size of each maze cell on the map
		int mapMargin = 20;
		int mapWidth = MAZE_SIZE * tileSize;
		int mapX = GetScreenWidth() - mapWidth - mapMargin;
		int mapY = mapMargin;
		// 1. Draw Map Background (Semi-transparent)
		DrawRectangle(mapX - 5, mapY - 5, mapWidth + 10, mapWidth + 10, Fade(BLACK,
			0.6f));
		// 2. Draw the Maze Grid
		for (int x = 0; x < MAZE_SIZE; x++) {
			for (int y = 0; y < MAZE_SIZE; y++) {
				Color cellColor = (maze[x][y] == 1) ? GRAY : Fade(WHITE, 0.1f);
				if (x == goal.x && y == goal.y) cellColor = GOLD;
				DrawRectangle(mapX + (x * tileSize), mapY + (y * tileSize), tileSize,
					tileSize, cellColor);
			}
		}
		// 3. Draw Player & Compass
		// Calculate map position based on 3D coordinates
		float playerMapX = mapX + (camera.position.x * tileSize);
		float playerMapY = mapY + (camera.position.z * tileSize);
		// The Compass Line: Uses the camera's forward vector
		Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target,
			camera.position));
		DrawLineV({ playerMapX, playerMapY },
			{ playerMapX + (forward.x * 15), playerMapY + (forward.z * 15) },
			RED);
		// Player Dot
		DrawCircle(playerMapX, playerMapY, 3, RED);
		// 4. Compass Letters (N, S, E, W)
		DrawText("N", mapX + mapWidth / 2 - 5, mapY - 15, 10, WHITE);
		DrawText("S", mapX + mapWidth / 2 - 5, mapY + mapWidth + 5, 10, WHITE);
		EndDrawing();
	}
	CloseWindow();
	return 0;
}