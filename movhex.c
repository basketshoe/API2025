#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_AIR_ROUTES 5
#define MIN_COST 0
#define MAX_COST 100
#define CACHE_SIZE 1009

typedef struct node {
    int xp, yp, xd, yd;
    int cost;
    // bool valid;
    struct node *next;
} Cache;

typedef struct {
    int total;
    Cache **array;
} CacheMap;

typedef struct {
    int target_x, target_y;
    int cost;
} AirRoute;

typedef struct {
    int ground_cost;
    AirRoute air_routes[MAX_AIR_ROUTES];
    int num_air_routes;
} Hexagon;

typedef struct {
    int x, y;
    int dist;
} HeapNode;

Hexagon *map = NULL;
int cols = 0, rows = 0;
CacheMap cache_map;
//int contoCache = 0;

/*void setNode(Cache *node, int xp, int yp, int xd, int yd, int cost) {
    node->xd = xd;
    node->yd = yd;
    node->xp = xp;
    node->yp = yp;
    node->cost = cost;
    node->valid = true;
    node->next = NULL;
}*/

void initCacheMap() {
    cache_map.total = CACHE_SIZE;
    cache_map.array = calloc(cache_map.total, sizeof(Cache *));
}

u_int32_t hashFunct(int xp, int yp, int xd, int yd) {
    u_int32_t hash = 2166136261u;

    hash ^= xp;
    hash *= 16777619u;
    hash ^= yp;
    hash *= 16777619u;
    hash ^= xd;
    hash *= 16777619u;
    hash ^= yd;
    hash *= 16777619u;

    return hash % CACHE_SIZE;
}

void insertCache(int xp, int yp, int xd, int yd, int cost) {
    u_int32_t idx = hashFunct(xp, yp, xd, yd);

    Cache *node = malloc(sizeof(Cache));

    //setNode(node, xp, yp, xd, yd, cost);

    node->xd = xd;
    node->yd = yd;
    node->xp = xp;
    node->yp = yp;
    node->cost = cost;
    //node->valid = true;
    node->next = NULL;

    if (cache_map.array[idx] == NULL) {
        cache_map.array[idx] = node;
    } else {
        // printf("COLLISION\n");
        node->next = cache_map.array[idx];
        cache_map.array[idx] = node;
    }
}

void invalidateCache() {
    Cache *curr = NULL, *temp = NULL;
    for (int i = 0; i < cache_map.total; i++) {
        if (cache_map.array[i] != NULL) {
            curr = cache_map.array[i];
            temp = curr;
            while (curr) {
                curr = curr->next;
                free(temp);
                temp = curr;
            }
        }
    }
    initCacheMap();
}

int searchRoute(int xp, int yp, int xd, int yd) {
    u_int32_t idx = hashFunct(xp, yp, xd, yd);

    //printf("ECCO %u %d\n", idx, cache_map.total);

    Cache *node = cache_map.array[idx];

    while (node != NULL) {
        if (xp == node->xp && yp == node->yp && xd == node->xd && yd == node->yd/* && node->valid*/) {
            return node->cost;
        }
        node = node->next;
    }
    return -1;
}

/*
void print_map() {
    if (map == NULL) {
        return;
    }

    for (int y = rows - 1; y >= 0; y--) {
        if (y % 2 != 0) {
            printf("\t");
        }

        for (int x = 0; x < cols; x++) {
            int idx = y * cols + x;
            printf("[%2d; %2d; %2d] ", x, y, map[idx].ground_cost);
        }
        printf("\n");
    }
}
*/

int hex_dist(int x1, int y1, int x2, int y2) {
    int q1 = x1 - (y1 - (y1 % 2))/2;
    int q2 = x2 - (y2 - (y2 % 2))/2;
    int r1 = y1;
    int r2 = y2;

    return (abs(q1 - q2) + abs(q1 + r1 - q2 - r2) + abs(r1 - r2)) / 2;
}

bool in_bound(int x, int y) {
    return (x >= 0 && x < cols) && (y >= 0 && y < rows);
}

int get_idx(int x, int y) {
    return y * cols + x;
}

void swap(HeapNode *a, HeapNode *b) {
    HeapNode temp = *a;
    *a = *b;
    *b = temp;
}

void heap_up(HeapNode *heap, int idx) {
    while (idx > 0 && heap[idx].dist < heap[(idx - 1) / 2].dist) {
        swap(&heap[idx], &heap[(idx - 1) / 2]);
        idx = (idx - 1) / 2;
    }
}

void heap_down(HeapNode *heap, int size, int idx) {
    int smallest = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;
    if (left < size && heap[left].dist < heap[smallest].dist) smallest = left;
    if (right < size && heap[right].dist < heap[smallest].dist) smallest = right;
    if (smallest != idx) {
        swap(&heap[idx], &heap[smallest]);
        heap_down(heap, size, smallest);
    }
}

void init() {
    int new_cols, new_rows;
    if (scanf("%d %d", &new_cols, &new_rows) != 2) return;

    invalidateCache();

    if (map) free(map);
    cols = new_cols;
    rows = new_rows;
    map = malloc(sizeof(Hexagon) * cols * rows);

    for (int i = 0; i < cols * rows; i++) {
        map[i].ground_cost = 1;
        map[i].num_air_routes = 0;
    }
    printf("OK\n");
}

void change_cost() {
    int x, y, v, raggio;
    if (scanf("%d %d %d %d", &x, &y, &v, &raggio) != 4) return;

    if (!in_bound(x, y) || raggio <= 0 || v < -10 || v > 10) {
        printf("KO\n");
        return;
    }

    invalidateCache();

    for (int j = y-raggio+1; j < y+raggio; j++) {
        if (j >= 0 && j < rows) {
            for (int i = x-raggio+1; i < x+raggio; i++) {
                if (i >= 0 && i < cols) {
                    int d = hex_dist(i, j, x, y);
                    if (d < raggio) {
                        float factor = (float)(raggio - d) / (float)raggio;
                        int increment = (int)__builtin_floorf((float)v * factor);

                        int idx = get_idx(i, j);
                        int new_c = map[idx].ground_cost + increment;
                        if (new_c < MIN_COST)
                            new_c = MIN_COST;
                        else if (new_c > MAX_COST)
                            new_c = MAX_COST;
                        map[idx].ground_cost = new_c;

                        for (int k = 0; k < map[idx].num_air_routes; k++) {
                            int new_air_c = map[idx].air_routes[k].cost + increment;
                            if (new_air_c < MIN_COST)
                                new_air_c = MIN_COST;
                            else if (new_air_c > MAX_COST)
                                new_air_c = MAX_COST;
                            map[idx].air_routes[k].cost = new_air_c;
                        }
                    }
                }
            }
        }
    }
    printf("OK\n");
}

void toggle_air_route() {
    int x1, y1, x2, y2;
    if (scanf("%d %d %d %d", &x1, &y1, &x2, &y2) != 4) return;

    if (!in_bound(x1, y1) || !in_bound(x2, y2)) {
        printf("KO\n");
        return;
    }

    int idx1 = get_idx(x1, y1);
    int existing_idx = -1;
    for (int i = 0; i < map[idx1].num_air_routes; i++) {
        if (map[idx1].air_routes[i].target_x == x2 && map[idx1].air_routes[i].target_y == y2) {
            existing_idx = i;
            break;
        }
    }

    if (existing_idx != -1) {
        for (int i = existing_idx; i < map[idx1].num_air_routes - 1; i++) {
            map[idx1].air_routes[i] = map[idx1].air_routes[i+1];
        }
        map[idx1].num_air_routes--;
        printf("OK\n");
        invalidateCache();
    } else {
        if (map[idx1].num_air_routes >= MAX_AIR_ROUTES) {
            printf("KO\n");
            return;
        }

        int sum = map[idx1].ground_cost;
        for (int i = 0; i < map[idx1].num_air_routes; i++) {
            sum += map[idx1].air_routes[i].cost;
        }
        int new_cost = sum / (map[idx1].num_air_routes + 1);

        int n = map[idx1].num_air_routes;
        map[idx1].air_routes[n].target_x = x2;
        map[idx1].air_routes[n].target_y = y2;
        map[idx1].air_routes[n].cost = (new_cost > MAX_COST) ? MAX_COST : new_cost;
        map[idx1].num_air_routes++;
        printf("OK\n");
        invalidateCache();
    }
}

void travel_cost() {
    int xp, yp, xd, yd;
    if (scanf("%d %d %d %d", &xp, &yp, &xd, &yd) != 4) return;

    if (!in_bound(xp, yp) || !in_bound(xd, yd)) {
        printf("-1\n");
        return;
    }
    
    if (xp == xd && yp == yd) {
        printf("0\n");
        return;
    }

    int result = searchRoute(xp, yp, xd, yd);
    if (result != -1) {
        printf("%d\n", result);
        // contoCache++;
        return;
    }

    int total_nodes = cols * rows;
    int *dist = malloc(sizeof(int) * total_nodes);
    if (!dist) return;
    for (int i = 0; i < total_nodes; i++) dist[i] = -1;
    
    HeapNode *heap = malloc(sizeof(HeapNode) * total_nodes * 1.25);
    if (!heap) {
        free(dist);
        return;
    }
    int heap_size = 0;

    int start_idx = get_idx(xp, yp);
    dist[start_idx] = 0;
    heap[heap_size++] = (HeapNode){xp, yp, 0};

    // int result = -1;
    while (heap_size > 0) {
        HeapNode current = *heap;

        *heap = heap[--heap_size];
        if (heap_size > 0) {
            heap_down(heap, heap_size, 0);
        }

        if (current.x == xd && current.y == yd) {
            result = current.dist;
            break;
        }

        int curr_idx = get_idx(current.x, current.y);
        if (dist[curr_idx] != -1 && current.dist > dist[curr_idx]) continue;

        int out_cost = map[curr_idx].ground_cost;
        if (out_cost > 0) {
            int *dx, *dy;

            int dx_even[] = {1, -1,  0, -1,  0, -1};
            int dy_even[] = {0,  0,  1,  1, -1, -1};

            int dx_odd[]  = {1, -1,  1,  0,  1,  0};
            int dy_odd[]  = {0,  0,  1,  1, -1, -1};

            if (current.y % 2 == 0) {
                dx = dx_even;
                dy = dy_even;
            } else {
                dx = dx_odd;
                dy = dy_odd;
            }

            for (int i = 0; i < 6; i++) {
                int nx = current.x + dx[i];
                int ny = current.y + dy[i];

                if (in_bound(nx, ny)) {
                    int n_idx = get_idx(nx, ny);
                    int new_d = current.dist + out_cost;
                    if (dist[n_idx] == -1 || new_d < dist[n_idx]) {
                        dist[n_idx] = new_d;
                        heap[heap_size++] = (HeapNode){nx, ny, new_d};
                        heap_up(heap, heap_size - 1);
                    }
                }
            }
        }

        for (int i = 0; i < map[curr_idx].num_air_routes; i++) {
            AirRoute r = map[curr_idx].air_routes[i];
            int n_idx = get_idx(r.target_x, r.target_y);
            int new_d = current.dist + r.cost;

            if (dist[n_idx] == -1 || new_d < dist[n_idx]) {
                dist[n_idx] = new_d;
                heap[heap_size++] = (HeapNode){r.target_x, r.target_y, new_d};
                heap_up(heap, heap_size - 1);
            }
        }
    }

    insertCache(xp, yp, xd, yd, result);

    if (result == 0) {
        printf("-1\n");
        return;
    }

    printf("%d\n", result);
    free(dist);
    free(heap);
}

int main() {
    char cmd[20];

    initCacheMap();
    if (scanf("%s", cmd) != EOF) {
        while (cmd[0] != EOF) {
            if (strncmp(cmd, "i", 1) == 0) init();
            else if (strncmp(cmd, "c", 1) == 0) change_cost();
            else if (strncmp(cmd, "to", 2) == 0) toggle_air_route();
            else if (strncmp(cmd, "tr", 2) == 0) travel_cost();
            // else if (strncmp(cmd, "p", 1) == 0) print_map();
            if (scanf("%s", cmd) != 1) break;
        }
    }
    // printf("Cache usata %d volte", contoCache);
    // if (map) free(map);
    return 0;
}
