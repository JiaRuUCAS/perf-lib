#ifndef _PROFILE_BITMAP_H_
#define _PROFILE_BITMAP_H_

#define BITMAP_MAX_BITS (1 << 20)

struct bitmap {
	bool is_mmap;
	unsigned int size;
	unsigned int n_bit;
	uint8_t array[];
};

struct bitmap *bitmap__new(unsigned int bit);

void bitmap__free(struct bitmap *map);

void bitmap__set_bit(struct bitmap *map, unsigned int pos);

void bitmap__clear_bit(struct bitmap *map, unsigned int pos);

bool bitmap__get_bit(struct bitmap *map, unsigned int pos);

void bitmap__clear(struct bitmap *map);

void bitmap__dump(struct bitmap *map);

#endif /* _PROFILE_BITMAP_H_ */
