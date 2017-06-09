#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>


#include <3ds.h>
#include <menu.h>

// The max number of items that can fit in the screen
#define MAX_ITEMS 28
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))


// Returns the position of the selected item or -1 if the menu was cancelled
int openMenu(menu_t* menu) {
    int i, numitems, pos, startpos;
    menu_t* cur_menu = menu;
    u32 keys;
    bool loop = true;

    while (loop) {
        pos = 0;
        startpos = 0;
        numitems = cur_menu->numitems;
        consoleClear();

        while (aptMainLoop() && loop) {
            hidScanInput();
            keys = hidKeysDown();

            if (keys & KEY_UP) {
                pos--;
                if (pos < startpos) {
                    if (pos >= 0) {
                        startpos--;
                    } else {
                        pos = numitems - 1;
                        startpos = MAX(0, numitems - MAX_ITEMS + 1);
                    }
                    consoleClear();
                }
            } else if (keys & KEY_DOWN) {
                pos++;
                if (pos >= MIN(numitems, startpos + MAX_ITEMS - 1)) {
                    if (pos >= numitems) {
                        pos = 0;
                        startpos = 0;
                    } else {
                        startpos++;
                    }
                    consoleClear();
                }
            } else if (keys & KEY_A) {
                if (cur_menu->items[pos].proc) {
                    int res = cur_menu->items[pos].proc();
                    if (res == -1) {
                        loop = false;
                        break;
                    }
				}	
                if (cur_menu->items[pos].child) {
                    cur_menu = cur_menu->items[pos].child;
                    break;
                } else {
                    loop = false;
                    break;
                }
            } else if (keys & KEY_B) {
                if (cur_menu->parent) {
                    cur_menu = cur_menu->parent;
                } else {
                    pos = -1;
                    loop = false;
                    break;
                }
                break;
			}
            printf("\x1b[;H\x1b[1m\x1b[36m%s:\n\x1b[0m", cur_menu->title);
//            printf("\x1b[92m%s:\n", cur_menu->title); //print in red 
			

            for (i = startpos; i < MIN(numitems, startpos + MAX_ITEMS - 1); i++) {
                char line[40];

                strncpy(line, cur_menu->items[i].text, 38);

                if (i == pos)
                    printf("\x1b[1m>");
                else
                    printf(" ");

                printf(line);
                printf("\x1b[0m\n");
            }
       }
    }
    return pos;
}

// Taken from github.com/smealum/3ds_hb_menu
static inline void unicodeToChar(char* dst, uint16_t* src, int max) {
    if(!src || !dst) return;
    int n = 0;
    while (*src && n < max - 1) {
        *(dst++) = (*(src++)) & 0xFF;
        n++;
    }
    *dst = 0x00;
}


FS_Archive sdmcArchive;


// TODO: Only show files that match the extension
int fileSelect(const char* message, char* path, const char* ext) {

    int i, pos = 0, item;
    menu_item_t files[64];
    char filenames[64][256];
    Handle dirHandle;
    uint32_t entries_read = 1;
    FS_DirectoryEntry entry;

    FSUSER_OpenArchive(&sdmcArchive, ARCHIVE_SDMC, (FS_Path){PATH_EMPTY, 1, (uint8_t*)"/"});

    // Scan directory. Partially taken from github.com/smealum/3ds_hb_menu
    Result res = FSUSER_OpenDirectory(&dirHandle, sdmcArchive, fsMakePath(PATH_ASCII, "/roms/neogeopocket/"));
    if (res) {
        consoleClear();
        printf("ERROR: %08X\n", res);
        printf("Unable to open sdmc:/roms/neogeopocket/\n");
        return -1;
    }

    for(i = 0; i < 32 && entries_read; i++) {
        memset(&entry, 0, sizeof(FS_DirectoryEntry));
        FSDIR_Read(dirHandle, &entries_read, 1, &entry);
        if(entries_read && !(entry.attributes & FS_ATTRIBUTE_DIRECTORY)) {
            //if(!strncmp("VB", (char*) entry.shortExt, 2)) {
            unicodeToChar(filenames[i], entry.name, 256);
            files[pos].text = filenames[i];
            pos++;
            //}
        }
    }

    FSDIR_Close(dirHandle);
    FSUSER_CloseArchive(sdmcArchive);

    item = openMenu(&(menu_t){message, NULL, pos, files});
    if (item >= 0 && item < pos)
        strcpy(path, "/roms/neogeopocket/"); // rom folder is 19 char exluding the final end string char
        strncpy(path+19, files[item].text, 237); //237 is 256 - 19 cars

    consoleClear();
    return item;
}



