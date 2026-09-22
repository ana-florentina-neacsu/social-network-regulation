#ifndef POST_H
#define POST_H

#define MAX_CONTENT_LENGTH 140

typedef struct {
    int id; // Unique post ID
    char content[MAX_CONTENT_LENGTH];
    float risk; // Risk level [0, 1]
} post;

#endif // POST_H