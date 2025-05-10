#ifndef HEADER_H
#define HEADER_H

typedef struct {
    char* key;
    char* value;

    void (*print)(struct header* hdr);
    void (*free)(struct header* hdr);
    
} Header;

#endif  // HEADER_H