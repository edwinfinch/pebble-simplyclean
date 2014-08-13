#define SETTINGS_KEY 0

Window *window;
GFont *h_n, *h_n_small;

InverterLayer *theme;

char timeBuffer[] = "00:00.";
char date_buffer[] = "26 Sept. 2014...";

TextLayer *time_layer, *date_layer, *battery_layer, *tis_layer, *update_at_a_glance;

int persistvalue;
int currentAppVer = 3;
bool currentlyGlancing = 0;
int versionChecked = 0;
GRect finish01, start02;
bool booted = 0;

typedef struct persist{
	uint8_t dateformat;
	bool btdisalert;
	bool btrealert;
	bool showdestext;
	uint8_t language;
	bool btdistheme;
	bool btcontheme;
	bool showbattext;
	bool showdate;
}__attribute__((__packed__)) persist;

persist settings = {
	.dateformat = 0,
	.btdisalert = 1,
	.btrealert = 1,
	.showdestext = 1,
	.language = 0,
	.btdistheme = 0,
	.btcontheme = 0,
	.showbattext = 0,
	.showdate = 0,
};

char *lang_itis[] = {
	"It is", "Es ist", "Es", "het is"
};

char *lang_dis[] = {
	"Bluetooth disconnected.", "Bluetooth getrennt", "BT desconectado.", "Bluetooth losgekoppeld.",
};

char *lang_con[] = {
	"Bluetooth reconnected.", "Bluetooth verbunden.", "BT reconectado.", "Bluetooth aangesloten.",
};

char glance_buffer[200];

/*
char *lang_on[] = {
	"on", "am", "en", "op"
};
*/