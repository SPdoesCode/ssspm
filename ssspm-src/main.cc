#include <vector>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <iostream>
#include <yaml-cpp/yaml.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include <git2.h>

using namespace std;

typedef enum {
	Installed,
	Uninstalled,
}PKGSTS;

typedef enum {
	Search,
	Inst,
}STYLE;

typedef enum {
	Install,
	Remove,
	Update,
	Sync,
}ACTION;

class Package {
public:

	string pkgname, author, pkgver, desc, src;

	vector<string> deps, install;
	
};

class Config {
public:

	string link, name;

};

class Global {
public:

	vector<string> pkgs;
		
};

void printPkgInfo(Global gbl, Package pkg, STYLE style); //
void checkRoot(void); //

Package parsePkg(string pkgname, Config cfg); //
Config parseConfig(void); //
Global parseGlobal(void); //

PKGSTS inGlobal(Global gbl, string pkg); //
void addGlobalPkg(string pkg); //
void removeGlobalPkg(string pkg); //
void updateGlobalPkgs(Global gbl, Config cfg); //

void installPackage(Package pkg, Global gbl);
void uninstallPackage(Package pkg, Global gbl);

void syncRepos(Config cfg);

int main(int argc, char *argv[]) {

	if (argc < 2) {
		cout << "ssspm Usage:\n";
		cout << "ssspm <action>\n";
		cout << "Actions are: \n\tin <package> | Installs a package\n\trm <package> | Uninstalls a package\n\tup | Updates the system\n\tsync | syncs the repo in the /ssspm/config\n";
		exit(1);
	}

	checkRoot();
	
	ACTION act;
	Package pkg;
	Config cfg = parseConfig();
	Global gbl = parseGlobal();
	
	string action = argv[1];
	
	if (action == "in") act = Install;
	else if (action == "rm") act = Remove;
	else if (action == "up") act = Update;
	else if (action == "sync") act = Sync;
	else {
		cout << "Unknown argument: " << action << "\n";
		cout << "ssspm Usage:\n";
		cout << "ssspm <action>\n";
		cout << "Actions are: \n   sh <package> | Looks for a package \n   in <package> | Installs a package\n   rm <package> | Uninstalls a package\n   up | Updates the system\n   sync | syncs the repos in the /ssspm/config\n";
		exit(1);
	}

	switch (act) {
		case Install:
			for (int i = 2; i < argc; i++) {
				pkg = parsePkg(argv[i-1], cfg);
				installPackage(pkg, gbl);
			}
		case Remove:
			for (int i = 2; i < argc; i++) {
				pkg = parsePkg(argv[i-1], cfg);
				uninstallPackage(pkg, gbl);
			}
		case Update:
			updateGlobalPkgs(gbl, cfg);
		case Sync:
			syncRepos(cfg);
	}
	
	exit(0);
}

void printPkgInfo(Global gbl, Package pkg, STYLE style) {
	switch (style) {
		case Inst:
			cout << "Package: " << pkg.pkgname << " || Version: " << pkg.pkgver << "\nDependancys:";
			for (string dep: pkg.deps) {
				cout << "   - " << dep << "\n";
			}
		case Search:
		
			cout << "Package: " << pkg.pkgname << "\nVersion: " << pkg.pkgver << "\nAuthor: " << pkg.author << "\nDescription: " << pkg.desc << "\nSource: " << pkg.src << "\nState: ";

			switch (inGlobal(gbl, pkg.pkgname)) {
				case Installed:
					cout << "Installed";
				case Uninstalled:
					cout << "Uninstalled";
			}

			cout << "\nDependancys:";

			for (string dep: pkg.deps) {
				cout << "   - " << dep << "\n";
			}
	}
}


void checkRoot(void) {

	if (geteuid() == 0) {
		
	} else {
		cout << "Please run the system as root (EUID is " << geteuid() << " not 0 (Called with geteuid)!)\n";
	}
	
}



Config parseConfig(void) {

	string configpath = "/ssspm/config";

	struct stat st;
	
	if (stat(configpath.c_str(), &st) == 0 && S_ISREG(st.st_mode)) {
		
	} else {
		cout << "ERROR}: Couldnt find config (" << configpath << ")!\n";
		exit(1);
	}

	try {
		YAML::Node config = YAML::LoadFile(configpath);

		Config cfg;
		string pno;

		pno = config["link"].as<string>();
		cfg.link = pno;

		pno = config["name"].as<string>();
		cfg.name = pno;

		return cfg;
		
	} catch (const YAML::Exception& e) {  

		std::cerr << "ERROR}: Error parsing YAML: " << e.what() << "\n";

		exit(1);
		
	} 
}

Package parsePkg(string pkgname, Config cfg) {

	string pkgpath = "/ssspm/repo/"+cfg.name+"/"+pkgname;

	struct stat st;

	if (stat(pkgpath.c_str(), &st) == 0 && S_ISREG(st.st_mode)) {
		cout << "Found package " << pkgname << "!\n";
	} else {
		cout << "ERROR}: Couldnt find package " << pkgname << "! (PKG PATH: " << pkgpath << ")!\n";
		exit(1);
	}
	
/*
	for (auto path: cfg.name) {
		pkgpath = "";
		pkgpath = "/ssspm/repo/"+path+pkgname;
	}
*/
	try {
	
		YAML::Node pkgopen = YAML::LoadFile(pkgpath);

		Package pkg;
		string pno;
		vector<string> pvo;
	
		pno = pkgopen["pkgname"].as<string>();
		pkg.pkgname = pno;

		pno = pkgopen["author"].as<string>();
		pkg.author = pno;

		pno = pkgopen["pkgver"].as<string>();
		pkg.pkgver = pno;

		pno = pkgopen["desc"].as<string>();
		pkg.desc = pno;

		pno = pkgopen["src"].as<string>();
		pkg.src = pno;

		pvo = pkgopen["deps"].as<vector<string>>();
		pkg.deps = pvo;

		pvo = pkgopen["install"].as<vector<string>>();
		pkg.install = pvo;
	
		return pkg;
	} catch (const YAML::Exception& e) {  

		std::cerr << "ERROR}: Error parsing YAML: " << e.what() << "\n";

		exit(1);
		
	} 
	
}

Global parseGlobal(void) {

	string globalfile = "/ssspm/var/global";

	struct stat st;
	
	if (stat(globalfile.c_str(), &st) == 0 && S_ISREG(st.st_mode)) {
		
	} else {
		cout << "ERROR}: Couldnt find global file! (PATH: " << globalfile << ")!\n";
		exit(1);
	}
	try {
	
		YAML::Node global = YAML::LoadFile(globalfile);

		Global gbl;
		vector<string> gbol;

		gbol = global["@Global"].as<vector<string>>();

		gbl.pkgs = gbol;

		return gbl;

	} catch (const YAML::Exception& e) {  

		std::cerr << "ERROR}: Error parsing YAML: " << e.what() << "\n";

		exit(1);
		
	}  
	
	
}



PKGSTS inGlobal(Global gbl, string pkg) {

	for (string pck: gbl.pkgs) {

		if (pck == pkg) {
			return Installed;
		}
		
	}
	
	return Uninstalled;
	
}

void addGlobalPkg(string pkg) {

	string globalfile = "/ssspm/var/global";
	
	struct stat st;
		
	if (stat(globalfile.c_str(), &st) == 0 && S_ISREG(st.st_mode)) {
		
	} else {
		cout << "ERROR}: Couldnt find global file! (PATH: " << globalfile << ")!\n";
		exit(1);
	}
	
	try {

		YAML::Node global = YAML::LoadFile(globalfile);

		global["@Global"].push_back(pkg);

		ofstream outFile(globalfile);

		outFile << global;

		outFile.close();

		cout << "Added " << pkg << " to @Global!\n";
			
	} catch (const YAML::Exception& e) {  

		std::cerr << "ERROR}: Error parsing YAML: " << e.what() << "\n";

		exit(1);
		
	} 
	
}

void removeGlobalPkg(string pkg) {

	string globalfile = "/ssspm/var/global";
		
	struct stat st;
			
	if (stat(globalfile.c_str(), &st) == 0 && S_ISREG(st.st_mode)) {
			
	} else {
		cout << "ERROR}: Couldnt find global file! (PATH: " << globalfile << ")!\n";
		exit(1);
	}
		
	try {
	
		YAML::Node global = YAML::LoadFile(globalfile);

		global["@Global"].remove(pkg);
	
		ofstream outFile(globalfile);
	
		outFile << global;
	
		outFile.close();
	
		cout << "Added " << pkg << " to @Global!\n";
				
	} catch (const YAML::Exception& e) {  

		std::cerr << "ERROR}: Error parsing YAML: " << e.what() << "\n";

		exit(1);
			
	}
	
}

void updateGlobalPkgs(Global gbl, Config cfg) {

	Package rpkg;
	
	for (string pkg: gbl.pkgs) {

		rpkg = parsePkg(pkg, cfg);
		installPackage(rpkg, gbl);
		
	}
	
}



void installPackage(Package pkg, Global gbl);

void uninstallPackage(Package pkg, Global gbl);

void syncRepos(Config cfg) {

	cout << "Syncing repo " << cfg.link << "...\n";
	
	git_libgit2_init();

	string ppath = "/ssspm/repo/"+cfg.name;

	const char *repourl = cfg.link.c_str();
	const char *path = ppath.c_str();

	git_repository *repo = NULL;

	int err = git_clone(&repo, repourl, path, NULL);

	if (err < 0) {

		const git_error *gerr = git_error_last();

		cout << "Failed to sync repo in config (link:" << cfg.link << ") with error " << err << ": " << gerr->message << "\n";
		
	} else {
		cout << "Successfully synced repo!\n";
	}
	
}
