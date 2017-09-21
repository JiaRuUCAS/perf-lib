#include "util.h"
#include "bitmap.h"


struct bitmap *bitmap__new(unsigned int bit)
{
	unsigned int real_bit = 0, size = 0;
	struct bitmap *map = NULL;

	real_bit = __roundup_2(bit);
	if (real_bit > BITMAP_MAX_BITS) {
		LOG_INFO("The required size is too big, limit to %u",
						BITMAP_MAX_BITS);
		real_bit = BITMAP_MAX_BITS;
	}

	size = sizeof(struct bitmap) + sizeof(uint8_t) * (real_bit >> 3);
	if (size >= PAGE_SIZE) {
		map = (struct bitmap*)mmap(NULL, size, PROT_READ | PROT_WRITE,
						MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
		if (map == MAP_FAILED) {
			LOG_ERROR("Failed to alloc %u-byte memory for bitmap", size);
			return NULL;
		}
		memset(map, 0, size);
		map->is_mmap = true;
	} else {
		map = (struct bitmap*)zalloc(size);
		if (map == NULL) {
			LOG_ERROR("Failed to alloc %u-byte memory for bitmap", size);
			return NULL;
		}
		map->is_mmap = false;
	}

	map->size = size;
	map->n_bit = real_bit;
	return map;
}

void bitmap__free(struct bitmap *map)
{
	if (map == NULL)
		return;

	if (map->is_mmap)
		munmap(map, map->size);
	else
		free(map);
	map = NULL;
}

void bitmap__set_bit(struct bitmap *map, unsigned int pos)
{
	if (pos >= map->n_bit) {
		LOG_ERROR("Wrong index %u, please use [0,%u]",
						pos, map->n_bit - 1);
		return;
	}
	map->array[pos >> 3] |= (1U << (pos & 7));
}

void bitmap__clear_bit(struct bitmap *map, unsigned int pos)
{
	if (pos >= map->n_bit) {
		LOG_ERROR("Wrong index %u, please use [0,%u]",
						pos, map->n_bit - 1);
		return;
	}
	map->array[pos >> 3] &= ~(1U << (pos & 7));
}

bool bitmap__get_bit(struct bitmap *map, unsigned int pos)
{
	if (pos >= map->n_bit) {
		LOG_ERROR("Wrong index %u, please use [0,%u]",
						pos, map->n_bit - 1);
		return false;
	}

	if ((map->array[pos >> 3] >> (pos & 7)) & 0x01)
		return true;
	return false;
}

void bitmap__clear(struct bitmap *map)
{
	if (map == NULL || map->array == NULL)
		return;

	memset(map->array, 0, sizeof(uint8_t) * (map->n_bit >> 3));
}

void bitmap__dump(struct bitmap *map)
{
	unsigned int i = 0, size = 0;
	char *str = NULL;

	if (map == NULL || map->array == NULL)
		return;

	size = map->n_bit >> 2;
	str = zalloc(size + 1);
	if (str == NULL)
		return;

	for (i = 0; i < (map->n_bit >> 3); i++) {
		sprintf(&str[i * 2], "%02x", map->array[i]);
	}
	str[size] = '\0';
	LOG_INFO("Bitmap (%u bits): %s", map->n_bit, str);
}
