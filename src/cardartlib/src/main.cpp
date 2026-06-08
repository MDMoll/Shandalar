#include "manalink.h"
#include "CardArtLib.h"
#include <ctype.h>
#include <cstring>
#include <utility>

namespace gdi = Gdiplus;

static std::unordered_map<std::string, std::unique_ptr<gdi::Image>> *cache;
static char** idToName;
static size_t numberOfCards = 0;
static CRITICAL_SECTION crit;
//gdiplus
static ULONG_PTR gdiplusToken;
static ULONG_PTR gdiplusBGThreadToken;
static gdi::GdiplusStartupInput gdiplusStartupInput;
static gdi::GdiplusStartupOutput gdiplusStartupOutput;
static bool gdiplus_available = false;
static bool images_counted = false;

static card_ptr_t* cards_ptr_shandalar_dll = NULL;

static std::unique_ptr<wchar_t[]> stringToWChar(const std::string& s)
{
	int len = s.length() + 1;
	std::unique_ptr<wchar_t[]> wcstring(new wchar_t[len]);
	mbstowcs(wcstring.get(), s.c_str(), len);
	return wcstring;
}

static bool read_exact(FILE* file, void* buffer, size_t size, size_t count)
{
	return fread(buffer, size, count, file) == count;
}

static void free_names(char** names, size_t count)
{
	if (!names)
		return;
	for (size_t i = 0; i < count; ++i)
		free(names[i]);
	free(names);
}

static std::string lowercase_ascii(const char* text)
{
	std::string result;
	if (!text)
		return result;
	for (const unsigned char* p = (const unsigned char*)text; *p; ++p)
		result += (char)tolower(*p);
	return result;
}

static bool loadDat(const char* fname){
	bool okchar[256] = {0};
	for (int i = 'a'; i <= 'z'; ++i)
		okchar[i] = true;
	for (int i = 'A'; i <= 'Z'; ++i)
		okchar[i] = true;
	for (int i = '0'; i <= '9'; ++i)
		okchar[i] = true;
	okchar[','] = true;
	okchar[' '] = true;
	okchar['\''] = true;
	okchar['-'] = true;

	FILE* f = fopen(fname, "rb");
	if (!f)
		return false;

	uint32_t card_count = 0;
	uint32_t string_data_size = 0;
	if (!read_exact(f, &card_count, 4, 1) || !read_exact(f, &string_data_size, 4, 1)){
		fclose(f);
		return false;
	}
	if (card_count == 0 || string_data_size == 0){
		fclose(f);
		return false;
	}

	char* stringData = (char*)malloc(string_data_size);
	card_ptr_t* cardData = (card_ptr_t*)malloc(sizeof(card_ptr_t) * card_count);
	char** names = (char**)calloc(card_count, sizeof(char*));
	if (!stringData || !cardData || !names){
		free(stringData);
		free(cardData);
		free_names(names, card_count);
		fclose(f);
		return false;
	}

	bool ok = read_exact(f, cardData, sizeof(card_ptr_t), card_count)
		&& read_exact(f, stringData, 1, string_data_size);
	fclose(f);
	if (!ok){
		free(stringData);
		free(cardData);
		free_names(names, card_count);
		return false;
	}

	for (unsigned int i = 0; i < card_count; i++) {
		int id = cardData[i].id;//*((int*)cardData + 38*i);
		int name_offset = (int)(cardData[i].full_name);//*((int*)cardData + 1 + 38*i));
		if (id < 0 || (unsigned int)id >= card_count || name_offset < 0 || (unsigned int)name_offset >= string_data_size)
			continue;

		char* name = stringData + name_offset;
		if (!memchr(name, 0, string_data_size - name_offset))
			continue;

		//replace characters
		for (char* p = name; *p; ++p)
		  if (!okchar[(int)(unsigned char)(*p)])
			*p = '_';
		names[id] = (char*)malloc(strlen(name) + 1);
		if (names[id])
			strcpy(names[id], name);
	}

	free(cardData);
	free(stringData);

	idToName = names;
	numberOfCards = card_count;
	return true;
}

static bool load_names(void)
{
	// Precondition: idToName == NULL

	HMODULE parent = GetModuleHandle(0);

	if (GetProcAddress(parent, "szDeckName")) { //manalink
		char* cards_dat = (char*)0x4D722E;
		return loadDat(cards_dat);
	} else if (GetModuleHandle("deck.exe")) { //deck.exe
		return loadDat("Cards.dat");
	} else { // shandalar
		return loadDat("Cards.dat");
	}
}

static std::string idToNameFun(int id, int version) {
	if (idToName == NULL && !load_names())
		return std::string();
	if (id < 0 || (size_t)id >= numberOfCards || !idToName[id])
		return std::string();

	std::string path = "CardArtManalink/";
	path += idToName[id];
	if (version > 0) {
		char version_buf[32];
		sprintf(version_buf, " (%d)", version);
		path += version_buf;
	}
	path += ".jpg";
	return path;
}

static void count_images(void)
{
	// Precondition: images_counted == false
	// Precondition: within critical section &crit

	images_counted = true;

	card_ptr_t* card_ptrs = NULL;

	HMODULE parent = GetModuleHandle(0);

	if (GetProcAddress(parent, "szDeckName")) { //manalink
		card_ptrs = *((card_ptr_t**)(0x73bae0));
	} else if (GetModuleHandle("deck.exe")) { //deck.exe
		// No need to do anything; deckbuilder always shows first image only
		return;
	} else { // shandalar
		if (!cards_ptr_shandalar_dll)
			return;
		card_ptrs = cards_ptr_shandalar_dll;
	}

	if (idToName == NULL && !load_names())
		return;

	std::unordered_map<std::string, int> name_to_csvid;
	for (unsigned int i = 0; i < numberOfCards; ++i) {
		const char* name = idToName[i];
		if (!name)
			continue;
		name_to_csvid[lowercase_ascii(name)] = i;
		card_ptrs[i].num_pics = 1;	// Always assume at least one image; it would have been corrected to 1 if initially set to 0 anyway
	}

	WIN32_FIND_DATAA find_data;
	HANDLE find = FindFirstFileA("CardArtManalink\\*.jpg", &find_data);
	if (find == INVALID_HANDLE_VALUE)
		return;

	do {
		if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			continue;

		/* boost::regex would be more concise, but this is about 10% faster when not disk-bound, and more importantly, doesn't bloat the dll file size.
		 * The old Boost filesystem dependency also made CardArtLib hard to rebuild with current MinGW toolchains. */
		std::string lower_filename = lowercase_ascii(find_data.cFileName);
		std::string cardname;
		const char* name = lower_filename.c_str();
		unsigned int num = 0;
		for (; *name; ++name){
			if (*name == '('){
				if (!cardname.empty() && cardname[cardname.size() - 1] == ' ' && *(name + 1) >= '1' && *(name + 1) <= '9'){
					cardname.resize(cardname.size() - 1);	// remove the last space before the parenthesis
					++name;
					num = atoi(name);
					while (*name && *name >= '0' && *name <= '9')
						++name;
					if (strcmp(name, ").jpg"))
						num = 0;	// didn't match epilogue
					break;
				} else
					break;	// ( found, but not after a space, or not followed by [1..9]
			}
			cardname += *name;
		}
		if (num == 0)	// no image number found, or failed after finding (
			continue;

		std::unordered_map<std::string, int>::const_iterator csvidit = name_to_csvid.find(cardname);
		if (csvidit == name_to_csvid.end())
			continue;

		int csvid = csvidit->second;
		++num;	// "Grizzly Bears (3).jpg" is actually the fourth image
		if (card_ptrs[csvid].num_pics < num)
			card_ptrs[csvid].num_pics = num;
	} while (FindNextFileA(find, &find_data));

	FindClose(find);
}

void Cardartlib_initialize_for_shandalar(card_data_t* real_cards_data, card_ptr_t* real_cards_ptr)
{
	// Avoid querying Shandalar.dll directly; it may not be present.
	(void)real_cards_data;
	cards_ptr_shandalar_dll = real_cards_ptr;
	EnterCriticalSection(&crit);
	if (!images_counted)
		count_images();	// during startup instead of waiting for first to display
	LeaveCriticalSection(&crit);
}

static bool file_exists(const char *filename)
{
	std::unique_ptr<wchar_t[]> wFilename = stringToWChar(filename);
	if (_waccess(wFilename.get(), 4) == 0) // check read permission
		return true;
	return false;
}

static bool ensure_gdiplus_started(void)
{
	if (gdiplus_available)
		return true;

	gdiplusStartupInput.GdiplusVersion = 1;
	gdiplusStartupInput.DebugEventCallback = NULL;
	gdiplusStartupInput.SuppressBackgroundThread = TRUE;
	gdiplusStartupInput.SuppressExternalCodecs = TRUE;

	if (GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, &gdiplusStartupOutput) != gdi::Ok || !gdiplusToken)
		return false;

	if (gdiplusStartupOutput.NotificationHook) {
		gdi::Status stat = gdiplusStartupOutput.NotificationHook(&gdiplusBGThreadToken);
		if (stat != gdi::Ok)
			gdiplusBGThreadToken = 0;
	}

	gdiplus_available = true;
	return true;
}

int LoadBigArt(int id, int version, LONG width, int height) {
	EnterCriticalSection(&crit);
	if (!ensure_gdiplus_started()) {
		LeaveCriticalSection(&crit);
		return 0;
	}
	if (!images_counted)
		count_images();
	std::string path = idToNameFun(id, version);
	if (path.empty()) {
		LeaveCriticalSection(&crit);
		return 0;
	}
	std::unordered_map<std::string, std::unique_ptr<gdi::Image>>::const_iterator it = cache->find(path);
	if (it == cache->end()) {
		if (!file_exists(path.c_str())) {
			LeaveCriticalSection(&crit);
			return 0;
		}
		std::unique_ptr<wchar_t[]> wPath = stringToWChar(path);
		std::unique_ptr<gdi::Image> tmpSurface(new gdi::Image(wPath.get()));
		if (tmpSurface->GetLastStatus() != gdi::Ok) {
			LeaveCriticalSection(&crit);
			return 0;
		}
		(*cache)[path] = std::move(tmpSurface);
	}
	LeaveCriticalSection(&crit);
	return 1; // 1 if the art is available
}

int LoadSmallArt(int id, int version, LONG width, int height) {
	return LoadBigArt(id, version, width, height);
}

void DestroyBigArt(int id, int version) {
	EnterCriticalSection(&crit);
	std::string path = idToNameFun(id, version);
	if (path.empty()) {
		LeaveCriticalSection(&crit);
		return;
	}
	std::unordered_map<std::string, std::unique_ptr<gdi::Image>>::iterator it = cache->find(path);
	if (it != cache->end()) {
		cache->erase(it);
		//MessageBox(0, path.c_str(), 0, MB_ICONSTOP | MB_SYSTEMMODAL);
	}
	LeaveCriticalSection(&crit);
} //Shandalar

void DestroySmallArt(int id, int version) {
	DestroyBigArt(id, version);
} //Shandalar

int IsBigArtRightSize(int id, int version, int width, int height) {
	return LoadBigArt(id, version, width, height);
}

int IsSmallArtRightSize(int id, int version, int width, int height) {
	return LoadBigArt(id, version, width, height);
}

void DestroyAllBigArts(void) {
	EnterCriticalSection(&crit);
	cache->clear();
	LeaveCriticalSection(&crit);
} //Shandalar

void DestroyAllSmallArts(void) {
	DestroyAllBigArts();
} //Shandalar

int IsBigArtIn(int id, int version) {
	return 0;
}

int IsSmallArtIn(int id, int version) {
	return 0;
}

int ReloadBigArtIfWrongSize(int id, int version, LONG width, int height) {
	return 0;
} //Shandalar

int ReloadSmallArtIfWrongSize(int id, int version) {
	return 0;
}

int DrawBigArt(HDC hdc, const RECT* rect, int id, int version) {
	if (!LoadBigArt(id, version, 0, 0)) {
		HBRUSH brush = CreateSolidBrush(RGB(211, 211, 211));
		FillRect(hdc, rect, brush);
		DeleteObject(brush);
		return 0;
	}
	EnterCriticalSection(&crit);
	gdi::Graphics graphics(hdc);

	std::string path = idToNameFun(id, version);
	gdi::Rect tmpRect(rect->left, rect->top, rect->right - rect->left, rect->bottom - rect->top);
	graphics.DrawImage((*cache)[path].get(), tmpRect);

	LeaveCriticalSection(&crit);
	return 1;
}

int DrawSmallArt(HDC hdc, const RECT* rect, int id, int version) {
	return DrawBigArt(hdc, rect, id, version);
}

static void init() {
	cache = new std::unordered_map<std::string, std::unique_ptr<gdi::Image>>;

	InitializeCriticalSection(&crit);

	images_counted = false;
}

static void deinit() {
	delete cache;
	cache = NULL;
	if (idToName) {
		for (unsigned int i = 0; i < numberOfCards; ++i)
			if (idToName[i])
				free(idToName[i]);
		free(idToName);
		idToName = NULL;
	}

	if (gdiplusBGThreadToken && gdiplusStartupOutput.NotificationUnhook) {
		gdiplusStartupOutput.NotificationUnhook(gdiplusBGThreadToken);
		gdiplusBGThreadToken = 0;
	}
	if (gdiplusToken) {
		gdi::GdiplusShutdown(gdiplusToken);
		gdiplusToken = 0;
	}
	gdiplus_available = false;

	DeleteCriticalSection(&crit);
}

int WINAPI DllMain(HINSTANCE hDllHandle, DWORD nReason, LPVOID reserved) {
	switch (nReason)
	{
	case DLL_PROCESS_ATTACH:
		init();
		return TRUE;
		break;
	case DLL_PROCESS_DETACH:
		DestroyAllBigArts();
		DestroyAllSmallArts();
		deinit();
		return TRUE;
		break;
	default:
		return TRUE;
	}
}
