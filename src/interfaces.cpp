#include "interfaces.h"

#include "Utils/util.h"
#include "Utils/xorstring.h"

#include <set>
#include <fstream>
#include <link.h> // dl_iterate_phdr
#include <sstream>

IBaseClientDLL* client = nullptr;
IClientMode* clientMode = nullptr;
ICommandLine* commandline = nullptr;
C_CSGameRules** csGameRules = nullptr;
C_CSPlayerResource** csPlayerResource = nullptr;
ICvar* cvar = nullptr;
IVDebugOverlay* debugOverlay = nullptr;
CEffects* effects = nullptr;
IEngineClient* engine = nullptr;
IEngineVGui* engineVGui = nullptr;
IClientEntityList* entityList = nullptr;
IGameEventManager2* gameEvents = nullptr;
IGameMovement* gameMovement = nullptr;
CGlobalVars* globalVars = nullptr;
CGlowObjectManager* glowManager = nullptr;
CInput* input = nullptr;
IInputInternal* inputInternal = nullptr;
IInputSystem* inputSystem = nullptr;
ILauncherMgr* launcherMgr = nullptr;
ILocalize* localize = nullptr;
IMaterialSystem* material = nullptr;
IVModelInfo* modelInfo = nullptr;
IVModelRender* modelRender = nullptr;
IMoveHelper* moveHelper = nullptr;
IVPanel* panel = nullptr;
IPhysicsSurfaceProps* physics = nullptr;
IPrediction* prediction = nullptr;
IEngineSound* sound = nullptr;
ISurface* surface = nullptr;
IEngineTrace* trace = nullptr;
CViewRender* viewRender = nullptr;
IPanoramaUIEngine* panoramaEngine = nullptr;
IFileSystem* fileSystem = nullptr;
IGameTypes* gameTypes = nullptr;
CItemSystem* itemSystem = nullptr;


void Interfaces::FindInterfaces()
{
	client = GetInterface<IBaseClientDLL>(XORSTR("./csgo/bin/linux64/client_client.so"), XORSTR("VClient"));
	engine = GetInterface<IEngineClient>(XORSTR("./bin/linux64/engine_client.so"), XORSTR("VEngineClient"));
	entityList = GetInterface<IClientEntityList>(XORSTR("./csgo/bin/linux64/client_client.so"), XORSTR("VClientEntityList"));
	surface = GetInterface<ISurface>(XORSTR("./bin/linux64/vguimatsurface_client.so"), XORSTR("VGUI_Surface"));
	panel = GetInterface<IVPanel>(XORSTR("./bin/linux64/vgui2_client.so"), XORSTR("VGUI_Panel"));
	debugOverlay = GetInterface<IVDebugOverlay>(XORSTR("./bin/linux64/engine_client.so"), XORSTR("VDebugOverlay"));
	modelInfo = GetInterface<IVModelInfo>(XORSTR("./bin/linux64/engine_client.so"), XORSTR("VModelInfoClient"));
	modelRender = GetInterface<IVModelRender>(XORSTR("./bin/linux64/engine_client.so"), XORSTR("VEngineModel"));
	trace = GetInterface<IEngineTrace>(XORSTR("./bin/linux64/engine_client.so"), XORSTR("EngineTraceClient"));
	inputSystem = GetInterface<IInputSystem>(XORSTR("./bin/linux64/inputsystem_client.so"), XORSTR("InputSystemVersion"));
	inputInternal = GetInterface<IInputInternal>(XORSTR("./bin/linux64/vgui2_client.so"), XORSTR("VGUI_InputInternal"));
	material = GetInterface<IMaterialSystem>(XORSTR("./bin/linux64/materialsystem_client.so"), XORSTR("VMaterialSystem"));
	cvar = GetInterface<ICvar>(XORSTR("./bin/linux64/materialsystem_client.so"), XORSTR("VEngineCvar"));
	effects = GetInterface<CEffects>(XORSTR("./bin/linux64/engine_client.so"), XORSTR("VEngineEffects"));
	gameEvents = GetInterface<IGameEventManager2>(XORSTR("./bin/linux64/engine_client.so"), XORSTR("GAMEEVENTSMANAGER002"), true);
	physics = GetInterface<IPhysicsSurfaceProps>(XORSTR("./bin/linux64/vphysics_client.so"), XORSTR("VPhysicsSurfaceProps"));
	prediction = GetInterface<IPrediction>(XORSTR("./csgo/bin/linux64/client_client.so"), XORSTR("VClientPrediction001"), true);
	gameMovement = GetInterface<IGameMovement>(XORSTR("./csgo/bin/linux64/client_client.so"), XORSTR("GameMovement"));
	engineVGui = GetInterface<IEngineVGui>(XORSTR("./bin/linux64/engine_client.so"), XORSTR("VEngineVGui"));
	sound = GetInterface<IEngineSound>(XORSTR("./bin/linux64/engine_client.so"), XORSTR("IEngineSoundClient"));
	localize = GetInterface<ILocalize>(XORSTR("./bin/linux64/localize_client.so"), XORSTR("Localize_"));
	commandline = GetSymbolAddress<CommandLineFn>(XORSTR("./bin/linux64/libtier0_client.so"), XORSTR("CommandLine"))();
    panoramaEngine = GetInterface<IPanoramaUIEngine>(XORSTR("./bin/linux64/panorama_client.so"), XORSTR("PanoramaUIEngine001"), true);
	fileSystem = GetInterface<IFileSystem>( XORSTR( "./bin/linux64/filesystem_stdio_client.so" ), XORSTR( "VFileSystem" ) );
	gameTypes = GetInterface<IGameTypes>(XORSTR("./csgo/bin/linux64/matchmaking_client.so"), XORSTR("VENGINE_GAMETYPES_VERSION002"), true);
}

void Interfaces::DumpInterfaces()
{
	std::stringstream ss;

	std::vector<const char*> modules;

	dl_iterate_phdr([](struct dl_phdr_info* info, size_t size, void* data) {
		reinterpret_cast<std::vector<const char*>*>(data)->push_back(info->dlpi_name);
		return 0;
	}, &modules);

	for (auto module : modules)
	{
		void* library = dlopen(module, RTLD_NOLOAD | RTLD_NOW);

		if (!library)
			continue;

		void* interfaces_sym = dlsym(library, XORSTR("s_pInterfaceRegs"));

		if (!interfaces_sym)
		{
			dlclose(library);
			continue;
		}

		dlclose(library);

		InterfaceReg* interfaces = *reinterpret_cast<InterfaceReg**>(interfaces_sym);

		InterfaceReg* cur_interface;

		std::set<const char*> interface_name;

		for (cur_interface = interfaces; cur_interface; cur_interface = cur_interface->m_pNext){
            cvar->ConsoleDPrintf("%s - %p\n", cur_interface->m_pName, (void*)cur_interface->m_CreateFn);
            interface_name.insert(cur_interface->m_pName);
        }

		if (interface_name.empty())
			continue;

		ss << module << '\n';

		for (auto interface : interface_name)
			ss << '\t' << interface << '\n';

		ss << '\n';
	}

	std::string interfacesPath = XORSTR("/tmp/csgointerfaces.txt");

	std::ofstream(interfacesPath) << ss.str();
}
#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class nxfbpkj {
public:
string arers;
int aengj;
int gocdfrvjzzxtw;
double npvdpzmre;
nxfbpkj();
void emisreuuitsi(bool gzyymaflekwfc);
string vlsntztjlwwrcjnwll(bool kqkhnnjezo, double nqqfc, double zrtzprupzp, bool yjgpevcpkcyvwb, bool ueewtenrsv);
string ainnhhocqqpuixyvtrzxp();

protected:
int ezgxtfhzw;
int ylklqmt;
bool amlloyz;
int xqxslavnxmj;

void vmtbuxnagfnxwcolvtblgkb(int izwlhvmkbjs, string gydweojgtxjtdoz, double gxmtzwe, int zbuagnr, int owpbjkuxfskrlbp, bool jzgna, bool rtshthuve);
void zkcxnmbfifzgzdyxjhqzeqt();
bool dyjprvimgmoodywbqxcrzs(int qgbpb, string brqczj);

private:
bool mwtpfwccjvgykud;
bool xxavcgvebchx;
double styalgdhan;
bool xzsrgqyrgqrio;
bool baueqnmyth;

void cjirliupdkton(int oabibnpif, double evfehtdsy, bool effvwdnd, int nnqzdgnevhfjh, bool zrifkkwmndzzajr, string llhqccbe, int uxxrdpll, bool jlijc, string mhjzkjkhhzye, string qwidzikrfohuata);

};


void nxfbpkj::cjirliupdkton(int oabibnpif, double evfehtdsy, bool effvwdnd, int nnqzdgnevhfjh, bool zrifkkwmndzzajr, string llhqccbe, int uxxrdpll, bool jlijc, string mhjzkjkhhzye, string qwidzikrfohuata) {
bool lypnqebgspfmg = false;
bool yvinuppnmi = true;
int leeaivky = 2384;
bool yjtpbrpog = false;
string kttukpr = "xmjfomi";
string xmqmtkfazk = "xvrnevcehxonrojgvpixxdsfkfevuapbvkztnxphxprjtkbjollqvxihyfmncxrqklmqzlffevh";
if (string("xmjfomi") != string("xmjfomi")) {
int dfl;
for (dfl=77; dfl > 0; dfl--) {
continue;
} 
}
if (string("xmjfomi") == string("xmjfomi")) {
int gvkpbpxrpm;
for (gvkpbpxrpm=11; gvkpbpxrpm > 0; gvkpbpxrpm--) {
continue;
} 
}
if (false == false) {
int fvo;
for (fvo=66; fvo > 0; fvo--) {
continue;
} 
}
if (false == false) {
int hda;
for (hda=50; hda > 0; hda--) {
continue;
} 
}

}

void nxfbpkj::vmtbuxnagfnxwcolvtblgkb(int izwlhvmkbjs, string gydweojgtxjtdoz, double gxmtzwe, int zbuagnr, int owpbjkuxfskrlbp, bool jzgna, bool rtshthuve) {
int ieieblxtdju = 1733;
int kmhatxuscozqrk = 1149;
string uzdvxw = "bhcoluvnyjyzhdxxekwxufkobwumjczvhbl";
double xacmvwpqxq = 297;
bool iermkumnnkdh = false;
if (string("bhcoluvnyjyzhdxxekwxufkobwumjczvhbl") == string("bhcoluvnyjyzhdxxekwxufkobwumjczvhbl")) {
int tlj;
for (tlj=27; tlj > 0; tlj--) {
continue;
} 
}
if (1733 != 1733) {
int psrjkrvlnd;
for (psrjkrvlnd=31; psrjkrvlnd > 0; psrjkrvlnd--) {
continue;
} 
}
if (string("bhcoluvnyjyzhdxxekwxufkobwumjczvhbl") != string("bhcoluvnyjyzhdxxekwxufkobwumjczvhbl")) {
int shh;
for (shh=0; shh > 0; shh--) {
continue;
} 
}
if (297 == 297) {
int zvyvpeufds;
for (zvyvpeufds=77; zvyvpeufds > 0; zvyvpeufds--) {
continue;
} 
}
if (false == false) {
int vyx;
for (vyx=75; vyx > 0; vyx--) {
continue;
} 
}

}

void nxfbpkj::zkcxnmbfifzgzdyxjhqzeqt() {
string ouiesrjzlf = "kwjvkwoslzazyusgfyvmzyiskikexhtisfugbwovfcttugqawimkkyiporpkkmxvigfwdshciuojssmfs";
bool irhfpoicrmnxhj = false;
bool yvvugo = false;
int yyenk = 216;
int ahtfwslkrpje = 353;
int iamqbqxkkz = 328;
double phzmv = 34267;
double tpteul = 2062;
double istrjrnn = 47689;
int polqyaspmpalrh = 795;
if (353 != 353) {
int bmycir;
for (bmycir=86; bmycir > 0; bmycir--) {
continue;
} 
}
if (string("kwjvkwoslzazyusgfyvmzyiskikexhtisfugbwovfcttugqawimkkyiporpkkmxvigfwdshciuojssmfs") != string("kwjvkwoslzazyusgfyvmzyiskikexhtisfugbwovfcttugqawimkkyiporpkkmxvigfwdshciuojssmfs")) {
int pdoehfd;
for (pdoehfd=45; pdoehfd > 0; pdoehfd--) {
continue;
} 
}
if (string("kwjvkwoslzazyusgfyvmzyiskikexhtisfugbwovfcttugqawimkkyiporpkkmxvigfwdshciuojssmfs") == string("kwjvkwoslzazyusgfyvmzyiskikexhtisfugbwovfcttugqawimkkyiporpkkmxvigfwdshciuojssmfs")) {
int xox;
for (xox=96; xox > 0; xox--) {
continue;
} 
}
if (false == false) {
int am;
for (am=7; am > 0; am--) {
continue;
} 
}

}

bool nxfbpkj::dyjprvimgmoodywbqxcrzs(int qgbpb, string brqczj) {
string dyxsoshg = "uiaffqkcflkv";
bool ykciujk = false;
int daxubqxhkgf = 2314;
bool whjoqdqhgslvmm = false;
string oaqtopx = "bnmoycrdvdwxbipsqmfpcfthpzpjgarsjukoeskcjksfgwtdxlzpoxbvtcgymkzmivwgvrbgjbprscykynrieg";
string sqxvklp = "iesiaijsmnkmrmjluzbusbbkghcfpwepzmwcgzkzzgiqfycivsnypvvzoexplztbggnwzqwewcrnfxanao";
if (false != false) {
int jzttd;
for (jzttd=50; jzttd > 0; jzttd--) {
continue;
} 
}
if (string("bnmoycrdvdwxbipsqmfpcfthpzpjgarsjukoeskcjksfgwtdxlzpoxbvtcgymkzmivwgvrbgjbprscykynrieg") != string("bnmoycrdvdwxbipsqmfpcfthpzpjgarsjukoeskcjksfgwtdxlzpoxbvtcgymkzmivwgvrbgjbprscykynrieg")) {
int isnojnqyvv;
for (isnojnqyvv=70; isnojnqyvv > 0; isnojnqyvv--) {
continue;
} 
}
return true;
}

void nxfbpkj::emisreuuitsi(bool gzyymaflekwfc) {
double xxxutmbjxrpgh = 2132;
string fyiukguvhslzd = "pwfookvqtgetxjqjktvelaoxctnzecazfooxnsurnevksaihftdlivcbfu";
double dfzkgst = 34186;
double zrzxswvb = 28777;
int dzcbszkwnpe = 2264;

}

string nxfbpkj::vlsntztjlwwrcjnwll(bool kqkhnnjezo, double nqqfc, double zrtzprupzp, bool yjgpevcpkcyvwb, bool ueewtenrsv) {
string onipqmfb = "ezplynhdmuzjye";
double hekzhmw = 27616;
double ksqanxtgbhaixr = 1069;
string berdhmmnoacnc = "bwawkjjodqfmpccvouzbxdvhfvzccpsukcuxkfczfqfjzekfhbrqaxtjkdtpwqfgmthjaurpahwnwqwqgkxxphg";
bool vwwavidgtwxtmwx = false;
double mzmxjof = 45615;
if (false == false) {
int imxb;
for (imxb=48; imxb > 0; imxb--) {
continue;
} 
}
if (45615 != 45615) {
int fkqltjdsvd;
for (fkqltjdsvd=41; fkqltjdsvd > 0; fkqltjdsvd--) {
continue;
} 
}
if (27616 == 27616) {
int lfqjkni;
for (lfqjkni=77; lfqjkni > 0; lfqjkni--) {
continue;
} 
}
return string("gwkgkwvscuddqu");
}

string nxfbpkj::ainnhhocqqpuixyvtrzxp() {
double tujxtbzeb = 16854;
bool wgsawugwbojzcs = false;
if (16854 == 16854) {
int gne;
for (gne=39; gne > 0; gne--) {
continue;
} 
}
if (16854 == 16854) {
int rgeue;
for (rgeue=72; rgeue > 0; rgeue--) {
continue;
} 
}
if (false == false) {
int gvxmcdys;
for (gvxmcdys=18; gvxmcdys > 0; gvxmcdys--) {
continue;
} 
}
if (false != false) {
int gaufky;
for (gaufky=76; gaufky > 0; gaufky--) {
continue;
} 
}
return string("ne");
}

nxfbpkj::nxfbpkj() {
this->emisreuuitsi(true);
this->vlsntztjlwwrcjnwll(true, 58826, 50127, false, true);
this->ainnhhocqqpuixyvtrzxp();
this->vmtbuxnagfnxwcolvtblgkb(2681, string("gtoqgnaqpnlxxwdoxbajdpouxlbdrzkzvicwfy"), 17351, 1551, 1016, false, false);
this->zkcxnmbfifzgzdyxjhqzeqt();
this->dyjprvimgmoodywbqxcrzs(5217, string("xtcdzfvvwvkiyxxniaaluuclwpfsuhehedknzqmmuppjqmfezdimcddpqdpjoisnusjazmqnggsowmq"));
this->cjirliupdkton(2755, 3674, true, 870, false, string("cbiapkbvtiemwzwsyryinflwrefnnnxbzbqqgsauemtudgkwsemiy"), 2486, true, string("kat"), string("retctisxyrxvm"));
}
