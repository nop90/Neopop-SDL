
typedef struct _menu_item_s {
    char* text;             // The text to display for the menu item
    int (*proc)(void);      // Called when the menu item is clicked
    struct _menu_s* child;  // Nested child menu
    int flags;              // Unused for now
    void* dp;               // Unused for now
} menu_item_t;

typedef struct _menu_s {
    char* title;            // The title of the menu
    struct _menu_s* parent; // The previous menu
    int numitems;           // The number of items in the menu
    struct _menu_item_s* items;
} menu_t;

int openMenu(menu_t* menu);

int fileSelect(const char* message, char* path, const char* ext);