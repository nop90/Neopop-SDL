#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>

#include "NeoPop-SDL.h"

#include <3ds.h>
#include <menu.h>

// The max number of items that can fit in the screen
#define MAX_ITEMS 27
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define LENGTH(array) (sizeof(array)/sizeof(array[0]))

extern int started;

char const *gui_Languages[] = {"Japanese","English"}; 
char const *gui_Colourmode[] = {"Grayscale","Color","Auto"}; 
char const *gui_YesNo[] = {"No", "Yes"};


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
                do {
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
				} while ( cur_menu->items[pos].flags==-1);
            } else if (keys & KEY_DOWN) {
                do {
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
				} while ( cur_menu->items[pos].flags==-1);
            } else if (keys & KEY_A) {
                if (cur_menu->items[pos].proc) {
                    int res = cur_menu->items[pos].proc();
					if (res == -1) {
                        loop = false;
                        break;
                    } else if ((res == 1) && cur_menu->parent) {
						cur_menu = cur_menu->parent;
                        break;
					}  
				} else if (cur_menu->items[pos].child) {
                    cur_menu = cur_menu->items[pos].child;
                    break;
                } else {
					loop = false;
					break;
				}  
				consoleClear();
            } else if (keys & KEY_B) {
                if (cur_menu->parent) {
                    cur_menu = cur_menu->parent;
                    break;
                 } else {
                    pos = -1;
                    loop = false;
                    break;
                }
                break;
			}
            printf("\x1b[;H\x1b[1m\x1b[36m%s:\n\n\x1b[0m", cur_menu->title);

            for (i = startpos; i < MIN(numitems, startpos + MAX_ITEMS - 1); i++) {
                char line[40];

                strncpy(line, cur_menu->items[i].text, 38);
				line[38]=0;
	
                if (i == pos)
                    printf("\x1b[1m>");
                else
                    printf(" ");

                printf(line);
				if (cur_menu->items[i].flags==1) printf("%i",*cur_menu->items[i].val);
				else if (cur_menu->items[i].flags==2) printf("%s",cur_menu->items[i].st[*cur_menu->items[i].val]);
                printf("\x1b[0m\n");
            }
			if(started) system_VBL();
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
menu_item_t files[100];
char filenames[100][256];

// TODO: Only show files that match the extension
int fileSelect(const char* message, char* path, const char* ext) {

    int i, pos = 0, item;
    Handle dirHandle;
    uint32_t entries_read = 1;
    FS_DirectoryEntry entry;

	for(i=0;i<100;i++) {
		filenames[i][0]='0';
		files[pos].text = NULL;
	}
	
    FSUSER_OpenArchive(&sdmcArchive, ARCHIVE_SDMC, (FS_Path){PATH_EMPTY, 1, (uint8_t*)"/"});

    // Scan directory. Partially taken from github.com/smealum/3ds_hb_menu
    Result res = FSUSER_OpenDirectory(&dirHandle, sdmcArchive, fsMakePath(PATH_ASCII, "/roms/neogeopocket/"));
    if (res) {
        consoleClear();
        printf("ERROR: %08X\n", res);
        printf("Unable to open /roms/neogeopocket/\n");
        return -1;
    }

    for(i = 0; i < 64 && entries_read; i++) {
        memset(&entry, 0, sizeof(FS_DirectoryEntry));
        FSDIR_Read(dirHandle, &entries_read, 1, &entry);
        if(entries_read && !(entry.attributes & FS_ATTRIBUTE_DIRECTORY)) {
            unicodeToChar(filenames[i], entry.name, 256);
            files[pos].text = filenames[i];
			files[pos].flags=0;
            pos++;
         }
    }

    FSDIR_Close(dirHandle);
    FSUSER_CloseArchive(sdmcArchive);

    item = openMenu(&(menu_t){message, NULL, pos, files});
    if (item >= 0 && item < pos)
        strcpy(path, "/roms/neogeopocket/"); // rom folder is 19 chars excluding the final \0 char
        strncpy(path+19, files[item].text, 237); //237 is 256 - 19 chars

    consoleClear();
    return item;
}

int file_loadrom(void) {
    int ret;
    char rompath[256];

	ret = fileSelect("Select ROM: [A] Select [B] Back", rompath, "ngc");
//		if (system_rom_load("/roms/neogeopocket/rom.ngc") == FALSE)
	if (ret>=0)	{
		if (system_rom_load(rompath) == FALSE) {
			fprintf(stderr, "wrong file format: no ROM loaded\n");
		} else {
			reset();
			return -1;
		}
	} else
		fprintf(stderr, "no ROM selected\n");
    return 0;
}

int options_frameskip(void) {
	system_frameskip_key++;
	if(system_frameskip_key>6) system_frameskip_key=1;
    return 0;
}

int options_saveslot(void) {
	state_slot = (state_slot+1)%10;
    return 0;
}


int options_sound(void) { 
	mute = !mute;
	return 0;
}

int options_save(void) {
	save_config("/3ds/neopop/neopop.cfg");
	return 1;
}

int emulation_reset(void) {
    reset();
    return -1;
}

int options_language(void) {
    language_english= language_english?0:1;
	return 0;
}

int options_color(void) {
    system_colour= (system_colour+1)%3;
	return 0;
}

int options_fullscreen(void) {
    system_graphics_fullscreen(!fs_mode);
	return 0;
}
int emulation_sstate(void) {
    system_state_save();
    return -1;
}

int emulation_lstate(void) {
    system_state_load();
    return -1;
}

int emu_exit(void) {
	do_exit = 1;
    return -1;
}

menu_item_t options_menu_items[] = {
    {"Frameskip        : ", options_frameskip, NULL, 1, NULL, &system_frameskip_key},
    {"Mute             : ", options_sound, NULL, 2, &gui_YesNo, &mute},
    {"Fullscreen       : ", options_fullscreen, NULL, 2, &gui_YesNo, &fs_mode},
    {"Color mode (*)   : ", options_color, NULL, 2, &gui_Colourmode, &system_colour},
    {"Sys Language (*) : ", options_language, NULL, 2, &gui_Languages,&language_english},
    {"", NULL, NULL, -1, NULL, NULL},
    {"Save options", options_save, NULL, 0, NULL, NULL},
    {"", NULL, NULL, -1, NULL, NULL},
    {"", NULL, NULL, -1, NULL, NULL},
    {"", NULL, NULL, -1, NULL, NULL},
    {"(*) Needs a system reset", NULL, NULL, -1, NULL, NULL}
};

menu_item_t main_menu_items[] = {
    {"Load ROM", file_loadrom, NULL, 0, NULL},
    {"Reset System", emulation_reset, NULL, 0, NULL, NULL},
    {"Save State", emulation_sstate, NULL, 0, NULL, NULL},
    {"Load State", emulation_lstate, NULL, 0, NULL, NULL},
    {"Saveslot   : ", options_saveslot, NULL, 1, NULL, &state_slot},
    {"Options", NULL, &options_menu, 0, NULL, NULL},
    {"", NULL, NULL, -1, NULL, NULL},
    {"Exit", emu_exit, NULL, 0, NULL, NULL}
};

menu_t main_menu = {
    "Main menu: [A] Select [B] Resume",
    NULL,
    LENGTH(main_menu_items),
    main_menu_items
};

menu_t options_menu  = {
    "Options: [A] Change [B] Back",
    &main_menu,
    LENGTH(options_menu_items),
    options_menu_items
};


