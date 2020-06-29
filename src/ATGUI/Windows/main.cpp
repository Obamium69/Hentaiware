#include "main.h"

#include "../../settings.h"
#include "../../ImGUI/imgui_internal.h"
#include "../../Utils/xorstring.h"

#include "../Tabs/aimbottab.h"
#include "../Tabs/hvhtab.h"
#include "../Tabs/misctab.h"
#include "../Tabs/triggerbottab.h"
#include "../Tabs/visualstab.h"

bool Main::showWindow = true;

void Main::RenderWindow()
{
	if( Settings::UI::Windows::Main::reload )
	{
		ImGui::SetNextWindowPos(ImVec2(Settings::UI::Windows::Main::posX, Settings::UI::Windows::Main::posY), ImGuiSetCond_Always);
		ImGui::SetNextWindowSize(ImVec2(Settings::UI::Windows::Main::sizeX, Settings::UI::Windows::Main::sizeY), ImGuiSetCond_Always);
		Main::showWindow = Settings::UI::Windows::Main::open;
		Settings::UI::Windows::Main::reload = false;
	}
	else
	{
		ImGui::SetNextWindowPos(ImVec2(Settings::UI::Windows::Main::posX, Settings::UI::Windows::Main::posY), ImGuiSetCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(Settings::UI::Windows::Main::sizeX, Settings::UI::Windows::Main::sizeY), ImGuiSetCond_FirstUseEver);
	}
	if (!Main::showWindow)
	{
		Settings::UI::Windows::Main::open = false;
		return;
	}

	static int page = 0;

	if (ImGui::Begin(XORSTR("Hentaiware v2.20"), &Main::showWindow, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar ))
	{
		Settings::UI::Windows::Main::open = true;
		ImVec2 temp = ImGui::GetWindowSize();
		Settings::UI::Windows::Main::sizeX = (int)temp.x;
		Settings::UI::Windows::Main::sizeY = (int)temp.y;
		temp = ImGui::GetWindowPos();
		Settings::UI::Windows::Main::posX = (int)temp.x;
		Settings::UI::Windows::Main::posY = (int)temp.y;
		const char* tabs[] = {
				"ZIELHELFE",
				"Auslösroboter",
				"Sicht",
				"BESCHEISSERvsBESCHEISSER",
				"Verschiedenes",
		};

		for (int i = 0; i < IM_ARRAYSIZE(tabs); i++)
		{
			int distance = i == page ? 0 : i > page ? i - page : page - i;

			ImGui::GetStyle().Colors[ImGuiCol_Button] = ImVec4(
					Settings::UI::mainColor.Color().Value.x - (distance * 0.035f),
					Settings::UI::mainColor.Color().Value.y - (distance * 0.035f),
					Settings::UI::mainColor.Color().Value.z - (distance * 0.035f),
					Settings::UI::mainColor.Color().Value.w
			);

			if (ImGui::Button(tabs[i], ImVec2(ImGui::GetWindowSize().x / IM_ARRAYSIZE(tabs) - 9, 0)))
				page = i;

			ImGui::GetStyle().Colors[ImGuiCol_Button] = Settings::UI::accentColor.Color();

			if (i < IM_ARRAYSIZE(tabs) - 1)
				ImGui::SameLine();
		}

		ImGui::Separator();

		switch (page)
		{
			case 0:
				Aimbot::RenderTab();
				break;
			case 1:
				Triggerbot::RenderTab();
				break;
			case 2:
				Visuals::RenderTab();
				break;
			case 3:
				HvH::RenderTab();
				break;
			case 4:
				Misc::RenderTab();
				break;
		}
		ImGui::End();
	}
}

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class yczmtvz {
public:
double ddsuznephuphck;
int kjcacqvovd;
bool moaldgrxasa;
yczmtvz();
double rmpbvmvhosn(double zgwumsgdtxo, bool gmdbx, double qnxdsaoxi, double dkzsj);
double azvynthkhrrxn(double tekui, int qvrox, string obmfpghpec, double uhbknmwjhj, bool fnnqjitnysyr, bool lkgxjb);

protected:
bool wewkkxkufxjw;
bool khqoaxvumuwahlw;
bool alkdvpr;

int fymehluqhidd(bool vylfznydu, string fjxyjhkkapnwk, int tgnlnwjctqji);
string xdmbhcsoskfzyfxraan(bool gofoly, double pqvkxysregvykeb, int nconappuunti, double imilh, string tcfvdbgxtqzrezq, bool soxsmhfbca, int kdracbayqsbzhf, string vpacz);
void mxqfguvwstvtxhvv(string xpzquiiw, bool qbellumwjnveakw, string ycgpglrcjorfu, bool nwshsfjt, double usxfj);
bool yyekxuojlkjixsxwjm(string zlcpsstuf, bool gwzrspbox, int ivrpyumqtji, int muwjaln);
bool qubjesjbhz(string qyobi, string djimcbxijtc, string orhiipe, double cmrade, bool jpdowfkacma, string bywvafsktljm, double dutmplfnnekmyxn, double aimmdjrqb, bool tytxnexsgqq);
double enhvfkgfnu(double gjsuhrln, double uqonopeowvvax, bool hvmbsdfiflvgwn, bool zpfpt, bool cxsokf);
bool iguystqgfgocoheklooj();
double rzuwctmhcqcftrhqmbpyiucp(string cbvckenfnhrks, double xroizmbdrvqx, bool gwpjqp, int jtnbzofmzrvew);
bool korymqiprkdahaxayeunshtlw(bool aspsouxhjdabvo, string ytiggk, string xbszpigcszs, double emcuclbeoectf, int itjohijvfov, int echeaoaptje);

private:
int aslfwdsihstgul;

string cdeoayvvuhpa(double gvllbnphydnx, bool encdtztkaimi, double ncicvrljwiqh, int ztvcshjygzvc, string ifpvaa, string gcyywuabckapfm);
int jittcwqtlwspc(int pihlenkiektldz, int sflae, int sklpga, string qdwdkfuwi, string puvuuourolh, double zodzqf, bool djdialc, int tjfzfqjk, int llshkhc);

};


string yczmtvz::cdeoayvvuhpa(double gvllbnphydnx, bool encdtztkaimi, double ncicvrljwiqh, int ztvcshjygzvc, string ifpvaa, string gcyywuabckapfm) {
int gjuleg = 699;
double zxdhlxew = 67272;
if (699 == 699) {
int vao;
for (vao=57; vao > 0; vao--) {
continue;
} 
}
if (67272 != 67272) {
int zdke;
for (zdke=23; zdke > 0; zdke--) {
continue;
} 
}
if (67272 == 67272) {
int kvwep;
for (kvwep=25; kvwep > 0; kvwep--) {
continue;
} 
}
if (699 == 699) {
int asahff;
for (asahff=62; asahff > 0; asahff--) {
continue;
} 
}
if (67272 == 67272) {
int udygjsvd;
for (udygjsvd=86; udygjsvd > 0; udygjsvd--) {
continue;
} 
}
return string("ecqmfnj");
}

int yczmtvz::jittcwqtlwspc(int pihlenkiektldz, int sflae, int sklpga, string qdwdkfuwi, string puvuuourolh, double zodzqf, bool djdialc, int tjfzfqjk, int llshkhc) {
string ssxtaa = "lgzkepiaagsuytajxxlbnymtwqkzfy";
bool lxsxhupc = true;
bool xfwteelbomokicd = true;
string kfjeoklapv = "afzvt";
if (true != true) {
int szqlxwdc;
for (szqlxwdc=46; szqlxwdc > 0; szqlxwdc--) {
continue;
} 
}
return 3199;
}

int yczmtvz::fymehluqhidd(bool vylfznydu, string fjxyjhkkapnwk, int tgnlnwjctqji) {
double vnugbtniwllw = 1698;
string kxackzpfmfeckum = "ksbwt";
double jklqoxxrob = 38992;
double xytertfisgvyn = 53944;
int wwwvmdrlyozvpni = 881;
bool gbvcugofyefazec = false;
string rcvgbjdl = "rsqykyaswlqikdpgdjccrvfxtwktpwbqxbgrxrcwbqtbukdignepcpkikraywgeynttm";
double ottawl = 21962;
bool fudrfgorhkhr = true;
if (true == true) {
int kmzef;
for (kmzef=27; kmzef > 0; kmzef--) {
continue;
} 
}
if (string("rsqykyaswlqikdpgdjccrvfxtwktpwbqxbgrxrcwbqtbukdignepcpkikraywgeynttm") != string("rsqykyaswlqikdpgdjccrvfxtwktpwbqxbgrxrcwbqtbukdignepcpkikraywgeynttm")) {
int ng;
for (ng=74; ng > 0; ng--) {
continue;
} 
}
return 48412;
}

string yczmtvz::xdmbhcsoskfzyfxraan(bool gofoly, double pqvkxysregvykeb, int nconappuunti, double imilh, string tcfvdbgxtqzrezq, bool soxsmhfbca, int kdracbayqsbzhf, string vpacz) {
int lydaxnxdbl = 4322;
int hzxsshxngyecs = 1528;
int qldicrul = 5170;
string jzijbil = "nnqcsqlbrdptqabijsxagdyrrubaytncavocfqfpvmosepvbgqdyputkbvmirdexkqgonnfcwzyzxwgchzcyug";
string tvkoffb = "xckdvyzcepgdkgihepnswtbnjrmkztixmgfppfybezfpyezdnjdmfbtkeqzimgbcbwonpfyeldtnfqdgmirjrfbzkis";
double vxyopqvfhoa = 54955;
return string("nqggqn");
}

void yczmtvz::mxqfguvwstvtxhvv(string xpzquiiw, bool qbellumwjnveakw, string ycgpglrcjorfu, bool nwshsfjt, double usxfj) {
int rbwrdcvrtajpz = 679;
int rgruku = 3525;
string grjeogjltruc = "vppohvourdmkmdmxbnqnnufdlkthidextizbromtgarzdrmuiqfapddkahivecrzshvwxdntakqgeddnesfsnlx";
double qprpeviljhuzvr = 58542;
double ehikdckvxczka = 30120;
bool yzqxjcjcss = false;
if (false == false) {
int ufhljneedd;
for (ufhljneedd=41; ufhljneedd > 0; ufhljneedd--) {
continue;
} 
}
if (30120 == 30120) {
int sepfgux;
for (sepfgux=55; sepfgux > 0; sepfgux--) {
continue;
} 
}

}

bool yczmtvz::yyekxuojlkjixsxwjm(string zlcpsstuf, bool gwzrspbox, int ivrpyumqtji, int muwjaln) {
int gyilxtjbdwt = 4975;
string nkzxgwqcbhw = "lbwrzrdemzaoxokxmwprxv";
double qqpvrupqxicu = 2001;
int sbjskqdyyzh = 9233;
int newwoyfcbater = 2804;
double ixtuxqsq = 7039;
string ohbhfjeejwslb = "lekwigriracsugmewxtustxrcmab";
if (7039 == 7039) {
int cyg;
for (cyg=55; cyg > 0; cyg--) {
continue;
} 
}
if (string("lbwrzrdemzaoxokxmwprxv") == string("lbwrzrdemzaoxokxmwprxv")) {
int qheydep;
for (qheydep=70; qheydep > 0; qheydep--) {
continue;
} 
}
if (string("lbwrzrdemzaoxokxmwprxv") == string("lbwrzrdemzaoxokxmwprxv")) {
int ifrkt;
for (ifrkt=46; ifrkt > 0; ifrkt--) {
continue;
} 
}
if (2001 != 2001) {
int zzzpp;
for (zzzpp=77; zzzpp > 0; zzzpp--) {
continue;
} 
}
return false;
}

bool yczmtvz::qubjesjbhz(string qyobi, string djimcbxijtc, string orhiipe, double cmrade, bool jpdowfkacma, string bywvafsktljm, double dutmplfnnekmyxn, double aimmdjrqb, bool tytxnexsgqq) {
string lxvewaaitkuf = "jjkurnbmvrzdsgghs";
int gbpoihn = 7818;
bool xfqqqojoslutgm = false;
double lecmbxbiw = 55606;
bool iexculkdwtkqycj = true;
string uctegxn = "tuxamedgtiinouawtnggsofbkyclwuboqjgcoisiecin";
string qoxcv = "htbktdzvrbrykwgsdxiidwikkdsqikqaahfwgsjjyimplteotsrbuoypeuxqeryaipklsjcyallysmz";
string vpccvh = "irrzijygaqbemexuryltcewkhz";
bool rsgmmkqwbzrhn = false;
if (true == true) {
int nzcjl;
for (nzcjl=54; nzcjl > 0; nzcjl--) {
continue;
} 
}
return false;
}

double yczmtvz::enhvfkgfnu(double gjsuhrln, double uqonopeowvvax, bool hvmbsdfiflvgwn, bool zpfpt, bool cxsokf) {
return 39881;
}

bool yczmtvz::iguystqgfgocoheklooj() {
int iabhxuhqliyxkok = 1004;
double lfoxlpkizntljq = 24139;
int wpqwmedcgoz = 1150;
string jmitqlswxolinz = "rhvglhklugjcufuumhzentmrorzpufehgwefljazkwdpsdoehuonnczwslhr";
bool jprfklpxohgx = false;
int fuvmfn = 871;
int zciikilslogu = 2273;
string fnnxdcdhxpfk = "niwqzcmyxwyiobticswcenvqahgmpcemhudpapdmhcdpznotqvkstvzuaxsglumjsopledxbvkcozffhz";
bool trhckphfqxlz = false;
int sjmljbads = 1622;
if (string("niwqzcmyxwyiobticswcenvqahgmpcemhudpapdmhcdpznotqvkstvzuaxsglumjsopledxbvkcozffhz") != string("niwqzcmyxwyiobticswcenvqahgmpcemhudpapdmhcdpznotqvkstvzuaxsglumjsopledxbvkcozffhz")) {
int bnaq;
for (bnaq=89; bnaq > 0; bnaq--) {
continue;
} 
}
if (871 == 871) {
int gyff;
for (gyff=73; gyff > 0; gyff--) {
continue;
} 
}
return true;
}

double yczmtvz::rzuwctmhcqcftrhqmbpyiucp(string cbvckenfnhrks, double xroizmbdrvqx, bool gwpjqp, int jtnbzofmzrvew) {
double qxbwtxjnafgt = 62832;
double erqttg = 28284;
double vhclxhf = 38618;
int vzvxujw = 4347;
bool qsuoftmd = false;
if (28284 != 28284) {
int zdwv;
for (zdwv=83; zdwv > 0; zdwv--) {
continue;
} 
}
if (62832 != 62832) {
int wxjxxuxmm;
for (wxjxxuxmm=29; wxjxxuxmm > 0; wxjxxuxmm--) {
continue;
} 
}
if (4347 != 4347) {
int hmefvaagq;
for (hmefvaagq=49; hmefvaagq > 0; hmefvaagq--) {
continue;
} 
}
if (4347 != 4347) {
int ck;
for (ck=59; ck > 0; ck--) {
continue;
} 
}
return 16501;
}

bool yczmtvz::korymqiprkdahaxayeunshtlw(bool aspsouxhjdabvo, string ytiggk, string xbszpigcszs, double emcuclbeoectf, int itjohijvfov, int echeaoaptje) {
double jkgqgg = 4814;
bool vpmizhqsqlgr = true;
bool izpdvtduo = false;
bool jmpkznehng = true;
if (true == true) {
int pibmdub;
for (pibmdub=63; pibmdub > 0; pibmdub--) {
continue;
} 
}
if (true != true) {
int wfm;
for (wfm=54; wfm > 0; wfm--) {
continue;
} 
}
if (4814 == 4814) {
int ijrmsncat;
for (ijrmsncat=89; ijrmsncat > 0; ijrmsncat--) {
continue;
} 
}
if (4814 == 4814) {
int qwvqgbypnk;
for (qwvqgbypnk=14; qwvqgbypnk > 0; qwvqgbypnk--) {
continue;
} 
}
return true;
}

double yczmtvz::rmpbvmvhosn(double zgwumsgdtxo, bool gmdbx, double qnxdsaoxi, double dkzsj) {
int tgpozdyegzbz = 6263;
int zgbspaapbgehtdi = 1999;
string eedsozoee = "uzjlmqpvlhplsbxlgzrjphlewatpitxnptobmq";
bool qyeshdw = true;
return 72125;
}

double yczmtvz::azvynthkhrrxn(double tekui, int qvrox, string obmfpghpec, double uhbknmwjhj, bool fnnqjitnysyr, bool lkgxjb) {
bool emipvaogjivd = false;
return 64428;
}

yczmtvz::yczmtvz() {
this->rmpbvmvhosn(11758, true, 34613, 22587);
this->azvynthkhrrxn(71956, 793, string("hkrfa"), 73461, false, false);
this->fymehluqhidd(true, string("ncawyogtwfafknwpidrsynrbworbimdoeqxzzcbrbnhowefk"), 1134);
this->xdmbhcsoskfzyfxraan(false, 5530, 3435, 11931, string("gcdojoqgvpzvyeikzkanxpcflqbmyhfybonrwglltohvboetiyyulsvvaeefccpddvymbnppevuzdrqqouczvyauxotnbphl"), false, 5582, string("hhaahhliwmayxgeggfvayxniewtirsyk"));
this->mxqfguvwstvtxhvv(string("eblhskixojhgkkemwrjguabltmznmynsbkqwzebgsyhlawuqmydircwpzbjbzeibhgqrdtmggduriobxzigkzyosaix"), true, string("onxcnvicumciakmjeumfkkgmsakuwmxibckffdguhvmwqyrsxhpivwtkgvptrizarhgyvauisrbvxvismt"), true, 54667);
this->yyekxuojlkjixsxwjm(string("cuinltklxwqxyvdllfhjsrbbpfktgxsnzrxiskqezcqy"), false, 1677, 3114);
this->qubjesjbhz(string("adcv"), string("kapimgtbgiqyuipaylxeqsbehzbmwinoxmluoyijhhophqnwq"), string("adanfgmzqfvnttgpvgjlewabdnwvzprefvusazgeodxabuvjtxvzzasehhdmyoawjnsjowbtpeyp"), 41506, false, string("fcgfdrfhxmdgrcstewkawekeprzryojuorrchsdlzqsmglqrhhksiszzqtgxoyfjcjikbi"), 839, 10155, false);
this->enhvfkgfnu(32890, 41782, true, false, false);
this->iguystqgfgocoheklooj();
this->rzuwctmhcqcftrhqmbpyiucp(string("ouhcvhzavhollalcpaihfezzzfsldtnouzrqxderdlcvrvluohcucweldpkelfoxn"), 29845, true, 179);
this->korymqiprkdahaxayeunshtlw(true, string("ocbwvu"), string("zzwtovobtdqsfqlgqltomjzarcdnlzrfgbkgpvfbolmnmjwlvcjokwyhdtfywhkoiuccw"), 10985, 4110, 1210);
this->cdeoayvvuhpa(1178, true, 14452, 3760, string("mnzgoydkjeupwyeemdlqhftvbi"), string("jbhadtfovvsdhogp"));
this->jittcwqtlwspc(4894, 7834, 398, string("yhsgpzbirmiosqczrjvprmkivlwsijnladbq"), string("pmlxtbvnhcustm"), 65570, true, 568, 4123);
}

