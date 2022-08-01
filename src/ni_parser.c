
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "ni_parser.h"
#include "libutil.h"

typedef struct
{
    uint16_t ni_version;
    uint16_t pack_version;
    uint32_t node_list_start;
    uint32_t node_size;
    uint32_t stage_nodes_count;
    uint32_t image_assets_count;
    uint32_t sound_assets_count;

} ni_file_t;

static ni_file_t gNiFile;

#define MAX_NB_NODES 100
static ni_node_t gNodes[MAX_NB_NODES];

static struct {
    uint32_t ri_size;
    uint32_t si_size;
    uint32_t li_size;
    uint8_t gRiBlock[512];
    uint8_t gSiBlock[512];
    uint8_t gLiBlock[512];
} pack;

#define DELTA 0x9e3779b9
#define MX (((z>>5^y<<2) + (y>>3^z<<4)) ^ ((sum^y) + (key[(p&3)^e] ^ z)))

static void btea_decode(uint32_t *v, uint32_t n, const uint32_t *key)
{
    uint32_t y, z, sum;
    uint32_t p, e;
    uint32_t rounds = 1 + 52/n;

    sum = rounds * DELTA;
    y = v[0];
    do {
        e = (sum >> 2) & 3;
        for (p=n-1; p>0; p--) {
            z = v[p-1];
            y = v[p] -= MX;
        }
        z = v[n-1];
        y = v[0] -= MX;
        sum -= DELTA;
    } while (--rounds);
}

static const uint32_t key[4] = {0x91bd7a0a, 0xa75440a9, 0xbbd49d6c, 0xe0dcc0e3};


void ni_set_ri_block(const uint8_t *data, uint32_t size)
{
    memcpy(pack.gRiBlock, data, size);
    pack.ri_size = size;
    uint32_t n = size / 4;
    btea_decode((uint32_t*)pack.gRiBlock, n, key);
}

void ni_set_si_block(const uint8_t *data, uint32_t size)
{
    memcpy(pack.gSiBlock, data, size);
    pack.si_size = size;
    uint32_t n = size / 4;
    btea_decode((uint32_t*)pack.gSiBlock, n, key);
}

void ni_set_li_block(const uint8_t *data, uint32_t size)
{
    memcpy(pack.gLiBlock, data, size);
    pack.li_size = size;
    uint32_t n = size / 4;
    btea_decode((uint32_t*) pack.gLiBlock, n, key);
}

void ni_decode_block512(uint8_t *data)
{
    btea_decode((uint32_t*) data, 128, key);
}

uint32_t ni_get_number_of_images()
{
    return pack.ri_size / 12;
}

void ni_get_image(char buffer[13], uint32_t index)
{
    uint32_t offset = index * 12;

    if (offset >= pack.ri_size)
    {
        offset = 0;
    }
    memcpy(buffer, &pack.gRiBlock[offset], 12);
    buffer[12] = '\0';
}

bool ni_get_node_info(uint32_t index, node_info_t *node)
{
    bool success = false;

    if (index < sizeof(gNodes))
    {
        node->current = &gNodes[index];
        // Copy sound file name
        uint32_t offset = node->current->sound_asset_index_in_si * 12;
        memcpy(node->si_file, &pack.gSiBlock[offset], 12);
        node->si_file[12] = '\0';

        // Copy image file name
        if (node->current->image_asset_index_in_ri != 0xFFFFFFFF)
        {
            offset = node->current->image_asset_index_in_ri * 12;
            memcpy(node->ri_file, &pack.gRiBlock[offset], 12);
            node->ri_file[12] = '\0';
        }
        else
        {
            // Pas d'image pour ce noeud
            node->ri_file[0] = '\0';
        }
    }
    else
    {
        printf("Bad node index\r\n");
    }

    return success;
}

// index en mot de 32 bits: il faut le multiplier par 4 pour avoir l'offet en octet
uint32_t ni_get_node_index_in_li(uint32_t index_in_li, uint32_t selected)
{
    uint32_t node_index = 0; // si erreur, on revient au point de départ
    if ((index_in_li * 4) < pack.li_size)
    {
        node_index = leu32_get(&pack.gLiBlock[(index_in_li + selected) * 4]);
    }
    else
    {
        printf("index_in_li too large\r\n");
    }
    return node_index;
}

void ni_parse_nodes(const uint8_t *data)
{
    if (gNiFile.stage_nodes_count <= MAX_NB_NODES)
    {
        for (uint32_t i = 0; i < gNiFile.stage_nodes_count; i++)
        {
            ni_node_t *n = &gNodes[i];
            const uint8_t *ptr = data + 512 + (gNiFile.node_size * i);
            n->image_asset_index_in_ri = leu32_get(ptr);
            ptr += 4;
            n->sound_asset_index_in_si = leu32_get(ptr);
            ptr += 4;
            n->ok_transition_action_node_index_in_li = leu32_get(ptr);
            ptr += 4;
            n->ok_transition_number_of_options = leu32_get(ptr);
            ptr += 4;
            n->ok_transition_selected_option_index = leu32_get(ptr);
            ptr += 4;
            n->home_transition_action_node_index_in_li = leu32_get(ptr);
            ptr += 4;
            n->home_transition_number_of_options = leu32_get(ptr);
            ptr += 4;
            n->home_transition_selected_option_index = leu32_get(ptr);
            ptr += 4;
            n->wheel = leu16_get(ptr);
            ptr += 2;
            n->ok = leu16_get(ptr);
            ptr += 2;
            n->home = leu16_get(ptr);
            ptr += 2;
            n->pause = leu16_get(ptr);
            ptr += 2;
            n->auto_play = leu16_get(ptr);
            ptr += 2;
            n->unknown = leu16_get(ptr);
        }
    }
}

bool ni_parser(const uint8_t *data)
{
    bool success = false;

    const uint8_t *ptr = data;

    gNiFile.ni_version = leu16_get(ptr);
    ptr += 2;
    gNiFile.pack_version = leu16_get(ptr);
    ptr += 2;
    gNiFile.node_list_start = leu32_get(ptr);
    ptr += 4;
    gNiFile.node_size = leu32_get(ptr);
    ptr += 4;
    gNiFile.stage_nodes_count = leu32_get(ptr);
    ptr += 4;
    gNiFile.image_assets_count = leu32_get(ptr);
    ptr += 4;
    gNiFile.sound_assets_count = leu32_get(ptr);

    success = true;

    ni_parse_nodes(data);

    return success;
}

void ni_dump()
{
    printf("NI version: %d \r\n", gNiFile.ni_version);
    printf("Pack version: %d \r\n", gNiFile.ni_version);
    printf("Node List start: %d \r\n", gNiFile.node_list_start);
    printf("Node size: %d \r\n", gNiFile.node_size);
    printf("Stage nodes count: %d \r\n", gNiFile.stage_nodes_count);
    printf("Image assets count: %d \r\n", gNiFile.image_assets_count);
    printf("Sound assets count: %d \r\n", gNiFile.sound_assets_count);
}
