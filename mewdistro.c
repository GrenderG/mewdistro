#include <gb/gb.h>
#include <gb/hardware.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <rand.h>
#include <gbdk/console.h>

#include "gen1.h"

#define PARTY_SIZE 6
#define POKE_SIZE 44
#define NAME_LEN 11
#define SELP_LEN 8
#define POKE_LEN POKE_SIZE * PARTY_SIZE
#define ORIG_LEN NAME_LEN * PARTY_SIZE
#define NICK_LEN NAME_LEN * PARTY_SIZE
#define FULL_LEN NAME_LEN + SELP_LEN + (POKE_SIZE * PARTY_SIZE) + (NAME_LEN * PARTY_SIZE) + (NAME_LEN * PARTY_SIZE)

#define SER_REG_DIR (*(uint8_t *)0xFF01)
#define SER_OPT_REG_DIR (*(uint8_t *)0xFF02)


const unsigned char mew_tiles[] =
{
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x01,0x03,0x06,0x01,0x08,0x09,0x09,
    0x02,0x12,0x1A,0x12,0x08,0x11,0x09,0x09,
    0x00,0x09,0x07,0x04,0x03,0x04,0x03,0x02,
    0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x01,0x03,0x03,0x07,0x00,0x00,
    0x30,0x78,0x66,0x47,0x50,0x60,0x28,0x30,
    0x20,0x30,0x00,0x20,0x25,0x46,0x46,0x49,
    0x47,0x4B,0x22,0x4A,0x23,0x23,0x21,0x21,
    0xD3,0xE0,0xF4,0x18,0xF8,0x7E,0x02,0x83,
    0x00,0x00,0x01,0x01,0x07,0x0F,0x03,0x11,
    0x13,0x11,0x87,0x8E,0xC7,0x46,0x2F,0xFE,
    0x96,0xFF,0x77,0x7F,0x13,0x1F,0x0B,0x0F,
    0x7F,0xFF,0xE7,0xFF,0xFF,0xFF,0x00,0x00,
    0xFC,0xFE,0x83,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x02,0x01,0x85,0x02,
    0x43,0x87,0x85,0x85,0x87,0x87,0x7A,0x86,
    0xFE,0x00,0x01,0x00,0x00,0x01,0x03,0x03,
    0xFC,0xFF,0x87,0xF8,0x08,0xF0,0x30,0xC0,
    0xC0,0x00,0x00,0x00,0x01,0x00,0x00,0x02,
    0x84,0x04,0x44,0x84,0xB8,0xC4,0xF3,0xFA,
    0xC1,0xE1,0x81,0x81,0x00,0x00,0x00,0x00,
    0x00,0x01,0x02,0x06,0x00,0x08,0x10,0x10,
    0x00,0x10,0x06,0x0E,0x00,0x01,0x00,0x00,
    0x1C,0x1C,0x64,0xE4,0xCC,0x44,0x14,0x0C,
    0x10,0x0C,0x18,0x08,0x88,0x84,0x84,0x44,
    0x8E,0x47,0x0F,0x44,0x09,0x07,0x18,0x08,
    0x30,0x10,0x40,0x60,0x80,0x80,0x00,0x00,
    0x80,0x80,0xC0,0x60,0xF8,0x18,0x7C,0x84,
    0xBE,0x42,0xFC,0x64,0xD8,0x58,0xC0,0x20,
    0x60,0x20,0x60,0x20,0xE0,0x20,0xD0,0x70,
    0x7C,0x8C,0x63,0x83,0xC9,0xC0,0x1F,0x3F,
    0x80,0xC0,0x20,0x30,0x08,0x08,0x08,0x04,
    0x0C,0x04,0x1C,0x02,0xFE,0xE2,0x0E,0x12,
    0x06,0x0A,0x0A,0x0A,0x0A,0x0A,0x08,0x0A,
    0x04,0x12,0x14,0x14,0x28,0x24,0x98,0xC8,
    0xF0,0x10,0xC0,0x60,0x00,0x80,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x80,0xC0,0x40,0x00,0x80,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

// It has +128 since we are loading Mew sprites starting at position 128
// to prevent collisions with the font sprites.
const unsigned char mew_map[] =
{
    0x94,0x94,0x94,0x8b,0x90,0x94,0x83,0x87,0x8c,0x91,
    0x80,0x84,0x88,0x8d,0x92,0x81,0x85,0x89,0x8e,0x94,
    0x82,0x86,0x8a,0x8f,0x93
};

enum connection_state_t connection_state = NOT_CONNECTED;
enum trade_state_t trade_state = INIT;
uint8_t INPUT_BLOCK[418];
uint8_t DATA_BLOCK[418];
int trade_pokemon = -1;
unsigned char name[11] = {
    pokechar_E,
    pokechar_U,
    pokechar_R,
    pokechar_O,
    pokechar_P,
    pokechar_E,
    pokechar_STOP_BYTE,
    pokechar_STOP_BYTE,
    pokechar_STOP_BYTE,
    pokechar_STOP_BYTE,
    pokechar_STOP_BYTE,
};
unsigned char nicknames[11] = {
    // Pokemon Nickname
    pokechar_M,
    pokechar_E,
    pokechar_W,
    pokechar_STOP_BYTE,
    pokechar_STOP_BYTE,
    pokechar_STOP_BYTE,
    pokechar_STOP_BYTE,
    pokechar_STOP_BYTE,
    pokechar_STOP_BYTE,
    pokechar_STOP_BYTE,
    pokechar_STOP_BYTE,
};

typedef struct TraderPacket {
    // Name must not exceed 10 characters + 1 STOP_BYTE
    // Any leftover space must be filled with STOP_BYTE
    unsigned char name[11];
    struct SelectedPokemon selected_pokemon;
    struct PartyMember pokemon[6];
    unsigned char original_trainer_names[6][11];
    unsigned char pokemon_nicknames[6][11];
} TraderPacket;

void print_log(const char *__s) {
    size_t max_length = 17; // 19 - ": " length.
    char __trimmed[18]; // max_length characters + 1 for null terminator.
    size_t len = strlen(__s);
    size_t i;

    // Copy characters up to max_length or until the end of the string.
    for (i = 0; i < max_length && i < len; ++i) {
        __trimmed[i] = __s[i];
    }

    // Fill the remaining space with empty spaces.
    for (; i < max_length; ++i) {
        __trimmed[i] = ' ';
    }

    // Null-terminate the string.
    __trimmed[max_length] = '\0';

    // Reset cursor position.
    gotoxy(1, 16);
    printf(": %s", __trimmed);
}

void party_member_to_bytes(struct PartyMember *pPartyMember, uint8_t *out) {
    uint8_t res[44] = {
        pPartyMember->pokemon,
        (uint8_t) (pPartyMember->current_hp >> 8),
        (uint8_t) (pPartyMember->current_hp & 0x00FF),
        pPartyMember->level,
        pPartyMember->status,
        pPartyMember->type1,
        pPartyMember->type2,
        pPartyMember->catch_rate_or_held_item,
        pPartyMember->move1,
        pPartyMember->move2,
        pPartyMember->move3,
        pPartyMember->move4,
        (uint8_t) (pPartyMember->original_trainer_id >> 8),
        (uint8_t) (pPartyMember->original_trainer_id & 0x00FF),
        (uint8_t) ((pPartyMember->experience & 0x00FF0000) >> 16),
        (uint8_t) ((pPartyMember->experience & 0x0000FF00) >> 8),
        (uint8_t) (pPartyMember->experience & 0x000000FF),
        (uint8_t) (pPartyMember->HP_ev >> 8),
        (uint8_t) (pPartyMember->HP_ev & 0x00FF),
        (uint8_t) (pPartyMember->attack_ev >> 8),
        (uint8_t) (pPartyMember->attack_ev & 0x00FF),
        (uint8_t) (pPartyMember->defense_ev >> 8),
        (uint8_t) (pPartyMember->defense_ev & 0x00FF),
        (uint8_t) (pPartyMember->speed_ev >> 8),
        (uint8_t) (pPartyMember->speed_ev & 0x00FF),
        (uint8_t) (pPartyMember->special_ev >> 8),
        (uint8_t) (pPartyMember->special_ev & 0x00FF),
        (uint8_t) (((pPartyMember->attack_iv & 0xF) << 4) | (pPartyMember->defense_iv & 0xF)),
        (uint8_t) (((pPartyMember->speed_iv & 0xF) << 4) | (pPartyMember->special_iv & 0xF)),
        pPartyMember->move1_pp,
        pPartyMember->move2_pp,
        pPartyMember->move3_pp,
        pPartyMember->move4_pp,
        pPartyMember->level,
        (uint8_t) (pPartyMember->max_hp >> 8),
        (uint8_t) (pPartyMember->max_hp & 0x00FF),
        (uint8_t) (pPartyMember->attack >> 8),
        (uint8_t) (pPartyMember->attack & 0x00FF),
        (uint8_t) (pPartyMember->defense >> 8),
        (uint8_t) (pPartyMember->defense & 0x00FF),
        (uint8_t) (pPartyMember->speed >> 8),
        (uint8_t) (pPartyMember->speed & 0x00FF),
        (uint8_t) (pPartyMember->special >> 8),
        (uint8_t) (pPartyMember->special & 0x00FF),
    };
    for (size_t i = 0; i < 44; i++) {
        out[i] = res[i];
    }
}

void selected_pokemon_to_bytes(struct SelectedPokemon *pSelectedPokemon, uint8_t *out) {
    uint8_t res[8] = {
        pSelectedPokemon->number,
        pSelectedPokemon->pokemon[0],
        pSelectedPokemon->pokemon[1],
        pSelectedPokemon->pokemon[2],
        pSelectedPokemon->pokemon[3],
        pSelectedPokemon->pokemon[4],
        pSelectedPokemon->pokemon[5],
        0xFF,
    };
    for (size_t i = 0; i < 8; i++) {
        out[i] = res[i];
    }
}

void trader_packet_to_bytes(struct TraderPacket *pTraderPacket, uint8_t *out) {
    uint8_t name_bytes[NAME_LEN];
    uint8_t selected_pokemon_bytes[SELP_LEN];
    uint8_t pokemon_bytes[POKE_LEN];
    uint8_t original_trainer_names_bytes[ORIG_LEN];
    uint8_t pokemon_nicknames_bytes[NICK_LEN];

    // Serialize the data
    for (size_t i = 0; i < NAME_LEN; i++) {
        name_bytes[i] = (uint8_t) pTraderPacket->name[i];
    }

    // Calculate the selected_pokemon and pokemon w/ stats and such
    for (size_t i = 0; i < PARTY_SIZE; i++) {
        uint8_t poke[POKE_SIZE];
        party_member_to_bytes(&pTraderPacket->pokemon[i], poke);

        // Full Party Data (all stats and such)
        for (size_t j = 0; j < POKE_SIZE; j++) {
            pokemon_bytes[(i * POKE_SIZE) + j] = (uint8_t) poke[j];
        }
    }
    selected_pokemon_to_bytes(&pTraderPacket->selected_pokemon, selected_pokemon_bytes);


    for (size_t i = 0; i < PARTY_SIZE; i++) {
        for (size_t j = 0; j < NAME_LEN; j++) {
            original_trainer_names_bytes[(i * NAME_LEN) + j] = (uint8_t) pTraderPacket->original_trainer_names[i][j];
        }
    }

    for (size_t i = 0; i < PARTY_SIZE; i++) {
        for (size_t j = 0; j < NAME_LEN; j++) {
            pokemon_nicknames_bytes[(i * NAME_LEN) + j] = (uint8_t) pTraderPacket->pokemon_nicknames[i][j];
        }
    }

    // Flatten the data
    uint8_t res[FULL_LEN + 3];

    for (size_t i = 0; i < NAME_LEN; i++) {
        res[i] = name_bytes[i];
    }
    for (size_t i = 0; i < SELP_LEN; i++) {
        res[i + NAME_LEN] = selected_pokemon_bytes[i];
    }
    for (size_t i = 0; i < POKE_LEN; i++) {
        res[i + NAME_LEN + SELP_LEN] = pokemon_bytes[i];
    }
    for (size_t i = 0; i < ORIG_LEN; i++) {
        res[i + NAME_LEN + SELP_LEN + POKE_LEN] = original_trainer_names_bytes[i];
    }
    for (size_t i = 0; i < NICK_LEN; i++) {
        res[i + NAME_LEN + SELP_LEN + POKE_LEN + ORIG_LEN] = pokemon_nicknames_bytes[i];
    }
    res[FULL_LEN] = 0x00;
    res[FULL_LEN + 1] = 0x00;
    res[FULL_LEN + 2] = 0x00;
    // Write the bytes to *out
    for (size_t i = 0; i < (FULL_LEN + 3); i++) {
        out[i] = res[i];
    }
}

// get a seed to be used for random generation by xoring values from ram which are pseudorandom on startup.
uint16_t get_ram_seed(void)
{
    uint16_t* p = (uint16_t*) 0xC000;
    uint16_t sum = 0;
    for (uint16_t i = 0; i < 0x1FFF; i++)
    {
        sum ^= p[i];
    }
    return sum;
}

void fill_pokemon_team(void)
{
    /**
     * Trader Packet Init
     */
    struct TraderPacket traderPacket;
    for (size_t i = 0; i < 11; i++) {
        traderPacket.name[i] = name[i];
    }

    struct SelectedPokemon *pSelectedPokemon = &traderPacket.selected_pokemon;
    pSelectedPokemon->number = 6;
    for (size_t i = 0; i < 6; i++) {
        pSelectedPokemon->pokemon[i] = MEW;
    }

    for (size_t i = 0; i < 6; i++) {
        struct PartyMember *pPartyMember = &traderPacket.pokemon[i];
        // Mimicking this Mew:
        // - https://github.com/projectpokemon/EventsGallery/tree/master/Released/Gen%201/Classic/International/2000%20Spanish%20Pok%C3%A9mon%20Championship
        // - https://projectpokemon.org/home/forums/topic/37431-gen-i-v-event-contributions-thread/?do=findComment&comment=254958
        // - https://www.math.miami.edu/~jam/azure/forum/tuff/ultimatebb.php?ubb=get_topic;f=6;t=000256
        // - https://projectpokemon.org/home/forums/topic/56562-uk-mew-from-julyaugust-2000/?do=findComment&comment=254955
        // - https://projectpokemon.org/home/forums/topic/37431-gen-i-v-event-contributions-thread/?do=findComment&comment=255300
        //
        // D-J one is not clear that it was legit, only EUROPE. Notes from Suloku about D-J one:
        //
        // El caso es que al final apareció el verdadero mew que repartieron en madrid, OT EUROPE.
        // Lo tenía un jugador que después de muchos años volvio a pokemon, y ya en la época tenía métodos de backup de
        // partida en gameboy, incluso estaba registrado en un foro de la época (inglés) donde había mensaje suyos
        // hablando de ir al torneo etc, todo verificado así que en principio ese mew es el que realmente repartieron.
        // El mew D-J...sigo pensando que efectivamente rondaba por madrid en el parque de atracciones, pero "repartido"
        // a base de clonaciones. O quien sabe, puede que alguien cambiara su mew hack al repartidor oficial del evento
        // y este se lo diera a otro jugador al ver que era un mew.

        pPartyMember->pokemon = MEW;
        pPartyMember->current_hp = 25;
        pPartyMember->max_hp = 25;
        pPartyMember->level = 5;
        pPartyMember->status = NONE;
        pPartyMember->type1 = PSYCHIC_TYPE;
        pPartyMember->type2 = PSYCHIC_TYPE; // If only one type, copy the first
        pPartyMember->catch_rate_or_held_item = 45; // R/G/B/Y (catch rate), G/S/C (held item), and Stadium (held item) use this byte differently
        pPartyMember->move1 = POUND;
        pPartyMember->move2 = 0x0;
        pPartyMember->move3 = 0x0;
        pPartyMember->move4 = 0x0;
        pPartyMember->original_trainer_id = (randw() % 65535) + 1;

        // -   Experience is complicated. You must look up the Pokemon you are trying to trade
        //      in the following table and apply the experience points that match the level.
        //      EXP LVL Table for gen 1: https://pwo-wiki.info/index.php/Generation_I_Experience_Charts
        //      That source was the best I could find for Gen 1. If you find another, submit a PR or open an issue and I'll fix it
        // -   Experience is a 24bit number, we will be dropping the MSB to acheive that
        pPartyMember->experience = 135;

        // Effort Values
        // These are very specific to the Pokemon and who they battled in the past or what vitamins they were fed
        // Luckily, these get recalculated when you level them up, or when you put them in a box and then put them back in your party
        // For this example, I will take the max value and scale it to the level (65535 * 0.40) = 26214
        pPartyMember->HP_ev = 0;
        pPartyMember->attack_ev = 0;
        pPartyMember->defense_ev = 0;
        pPartyMember->speed_ev = 0;
        pPartyMember->special_ev = 0;

        // IVs are a 4 bit number, so the max value is 15 (0-15 = 0b0000-0b1111 = 0x0-0xF)
        // These have been broken out for legibility, but will be condensed to only 2 bytes
        pPartyMember->attack_iv = 10;
        pPartyMember->defense_iv = 1;
        pPartyMember->speed_iv = 12;
        pPartyMember->special_iv = 5;

        pPartyMember->move1_pp = 35;
        pPartyMember->move2_pp = 0;
        pPartyMember->move3_pp = 0;
        pPartyMember->move4_pp = 0;

        pPartyMember->attack = 100;
        pPartyMember->defense = 100;
        pPartyMember->speed = 100;
        pPartyMember->special = 100;
    }

    for (size_t i = 0; i < 6; i++) {
        for (size_t j = 0; j < 11; j++) {
            traderPacket.original_trainer_names[i][j] = name[j];
        }
    }
    for (size_t i = 0; i < 6; i++) {
        for (size_t j = 0; j < 11; j++) {
            traderPacket.pokemon_nicknames[i][j] = nicknames[j];
        }
    }

    trader_packet_to_bytes(&traderPacket, DATA_BLOCK);
}

uint8_t handle_byte(uint8_t in, size_t *counter) {
    uint8_t out[1];

    switch (connection_state)
    {
        case NOT_CONNECTED:
            switch (in)
            {
                case PKMN_MASTER:
                    out[0] = PKMN_SLAVE;
                    break;
                case PKMN_BLANK:
                    out[0] = PKMN_BLANK;
                    break;
                case PKMN_CONNECTED:
                    connection_state = CONNECTED;
                    out[0] = PKMN_CONNECTED;
                    break;
            }
            break;

        case CONNECTED:
            switch (in)
            {
                case PKMN_CONNECTED:
                    out[0] = PKMN_CONNECTED;
                    break;
                case PKMN_TRADE_CENTRE:
                    // No byte known, just move on the next case
                    connection_state = TRADE_CENTRE;
                    break;
                case PKMN_COLOSSEUM:
                    // No byte known, just move on the next case
                    // This case is not built out and I have no intention to do it
                    connection_state = COLOSSEUM;
                    break;
                case PKMN_BREAK_LINK:
                case PKMN_MASTER:
                    connection_state = NOT_CONNECTED;
                    out[0] = PKMN_BREAK_LINK;
                    break;

                default:
                    out[0] = in;
                    break;
            }
            break;

        case TRADE_CENTRE:
            if(trade_state == INIT && in == 0x00) {
                print_log("Waiting...");
                // Fill team on each init, this way Pokémon ID is regenerated if it's random (otherwise this
                // can be moved somewhere else to only be called once).
                fill_pokemon_team();

                trade_state = READY;
                out[0] = 0x00;
            } else if(trade_state == READY && in == 0xFD) {
                trade_state = DETECTED;
                out[0] = 0xFD;
            } else if(trade_state == DETECTED && in != 0xFD) {
                out[0] = in;
                trade_state = DATA_TX_RANDOM;
            } else if(trade_state == DATA_TX_RANDOM && in == 0xFD) {
                print_log("Sending data...");
                trade_state = DATA_TX_WAIT;
                out[0] = 0xFD;
                (*counter) = 0;
            } else if (trade_state == DATA_TX_WAIT && in == 0xFD) {
                out[0] = 0x00;
            } else if(trade_state == DATA_TX_WAIT && in != 0xFD) {
                (*counter) = 0;
                // send first byte
                out[0] = DATA_BLOCK[(*counter)];
                INPUT_BLOCK[(*counter)] = in;
                trade_state = DATA_TX;
                (*counter)++;
            } else if(trade_state == DATA_TX) {
                out[0] = DATA_BLOCK[(*counter)];
                INPUT_BLOCK[(*counter)] = in;
                (*counter)++;
                if((*counter) == 418) {
                    trade_state = DATA_TX_PATCH;
                }
            } else if(trade_state == DATA_TX_PATCH && in == 0xFD) {
                (*counter) = 0;
                out[0] = 0xFD;
            } else if(trade_state == DATA_TX_PATCH && in != 0xFD) {
                out[0] = in;
                (*counter)++;
                if((*counter) == 197) {
                    trade_state = TRADE_WAIT;
                }
            } else if(trade_state == TRADE_WAIT && (in & 0x60) == 0x60) {
                if (in == 0x6f) {
                    print_log("Waiting...");
                    trade_state = READY;
                    out[0] = 0x6f;
                } else {
                    print_log("Selecting...");
                    out[0] = 0x60;
                    trade_pokemon = in - 0x60;
                }
            } else if(trade_state == TRADE_WAIT && in == 0x00) {
                out[0] = 0;
                trade_state = TRADE_DONE;
            } else if(trade_state == TRADE_DONE && (in & 0x60) == 0x60) {
                out[0] = in;
                if (in  == 0x61) {
                    print_log("Waiting...");
                    trade_pokemon = -1;
                    trade_state = TRADE_WAIT;
                } else {
                    trade_state = DONE;
                }
            } else if(trade_state == DONE && in == 0x00) {
                print_log("Finishing...");
                out[0] = 0;
                trade_state = INIT;
            } else {
                out[0] = in;
            }
            break;

        default:
            out[0] = in;
            break;
    }

    return out[0];
}

uint8_t linkcable_trade_byte(uint8_t data) {
    SB_REG = data;
    SC_REG = SIOF_XFER_START | SIOF_CLOCK_INT;
    while (SC_REG & SIOF_XFER_START);
    return SB_REG;
}

void main(void)
{
    disable_interrupts();

    // Read from RAM to generate the seed (from 0xC000 to 0xDFFF) for later pseudo-random TID generation.
    initrand(get_ram_seed());

    puts("\n  MEW DISTRIBUTION");
    puts("     OT/ EUROPE\n\n\n\n\n\n\n\n\n\n\n");
    puts("       by @GrenderG");

    print_log("Ready");

    // Load Mew tiles starting at position 128.
    set_bkg_data(128, 20, mew_tiles);
    // Draw Mew figure in the middle of the screen (more or less).
    set_bkg_tiles(7, 6, 5, 5, mew_map);

    size_t trade_counter = 0;
    uint8_t _in, _out = 0x00;
    while(TRUE) {
        _in = linkcable_trade_byte(_out);

        // We do this because the serial interface halts operations on
        // the other Game Boy and freezes the game, so we need to give it
        // time to think.
        __asm
            .rept 16
                push hl
                pop  hl
            .endm
        __endasm;

        _out = handle_byte(_in, &trade_counter);
    }
}
