#define _CRT_SECURE_NO_WARNINGS

#include "stdafx.h"
#include "string"
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <windows.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <tchar.h>
#include <direct.h>
#include <filesystem>

using namespace std;
namespace fs = std::experimental::filesystem;

class File {
public:
	File() {}
	string name;
	unsigned char *data;
	long size;
};

vector<string> split(const string &s, char delim) {
	vector<string> result;
	stringstream ss(s);
	string item;

	while (getline(ss, item, delim)) {
		if (item != "")
			result.push_back(item);
	}

	return result;
}

class Folder {
public:
	string name;
	Folder *parent = NULL;
	vector<File *> files;
	vector<Folder *> children;
	Folder() {
		parent = NULL;
	}
	Folder(Folder *parent) : parent(parent)
	{
	}
	void add_child(string fName) {
		if (!checkExists(fName, this) && isValidName(fName)) {
			Folder *child = new Folder();
			child->name = fName;
			child->parent = this;
			children.push_back(child);
		}
		else if (!isValidName(fName))
			cout << "Directory name can't have back or forward slashes" << endl;
		else
			cout << "Error: File or folder already exists with the same name" << endl;
	}

	void add_file(string fName, string data) {
		if (!checkExists(fName, this) && isValidName(fName)) {
			File *file = new File();
			file->name = fName;
			int n = data.length();
			// file->data = new unsigned char[n + 1];
			// strcpy(file->data, data.c_str());
			// file->data = data;
			files.push_back(file);
		}
		else if (!isValidName(fName))
			cout << "File name can't have back or forward slashes" << endl;
		else
			cout << "Error: File or folder already exists with the same name" << endl;

	}

	void add_ext_file(string fName, string path) {
		if (!checkExists(fName, this) && isValidName(fName)) {
			const char *path2 = path.c_str();
			FILE *f = fopen(path2, "rb");
			size_t res;
			File *file = new File();
			if (f == NULL) {
				perror("error");
			}
			else {
				fseek(f, 0, SEEK_END);
				long fsize = ftell(f);
				file->size = fsize;
				rewind(f);
				unsigned char *dd = new unsigned char[fsize + 1];
				res = fread(dd, 1, fsize, f);
				fclose(f);
				dd[fsize] = 0;
				file->data = new unsigned char[fsize + 1];
				file->data = dd;
				file->name = fName;
				files.push_back(file);
				if (res != fsize) {
					cout << "read error" << endl;
				}
			}
		}
		else if (!isValidName(fName))
			cout << "File name can't have back or forward slashes" << endl;
		else
			cout << "Error: File or folder already exists with the same name" << endl;
	}
	bool checkExists(string fName, Folder *parent) {
		// Check if folder or file exists with the same name
		for (vector<Folder*>::iterator it = parent->children.begin(); it != parent->children.end(); ++it) {
			Folder *x = *it;
			if (x->name == fName) {
				return true;
			}
		}

		for (vector<File*>::iterator it = parent->files.begin(); it != parent->files.end(); ++it) {
			File *x = *it;
			if (fName == x->name) {
				return true;
			}
		}

		return false;
	}
	bool checkExistsBeforeCP(string fName, Folder *f) {
		for (vector<File*>::iterator it = f->files.begin(); it != f->files.end(); ++it) {
			File *x = *it;
			if (fName == x->name) {
				return true;
			}
		}

		return false;
	}
	void ls() {
		cout << children.size() << " Dir(s)" << endl;
		cout << files.size() << " File(s)" << endl;
		for (vector<Folder*>::iterator it = children.begin(); it != children.end(); ++it) {
			Folder *x = *it;
			cout << " - " << x->name << endl;
		}

		for (vector<File*>::iterator it = files.begin(); it != files.end(); ++it) {
			File *x = *it;
			cout << " - " << x->name << "\t" << x->size << endl;
		}
	}

	Folder * cd(string fName) {
		bool found = false;
		if (fName == ".." && parent != NULL) {
			return parent;
		}

		Folder *res = getByPath(fName);

		if (res == NULL) {
			cout << "Directory does not exist" << endl;
			return this;
		}
		return res;
	}

	void rd(string fName) {
		bool found = false;
		int position = 0;
		Folder *f = getByPath(fName);
		if (f != NULL && f->parent != NULL) {
			Folder *p = f->parent;
			p->children.erase(remove(p->children.begin(), p->children.end(), f), p->children.end());
		}
		else
			cout << "Directory does not exist" << endl;
	}
	void rm(string fName) {
		bool found = false;
		size_t idx = fName.find('/');
		Folder *xx = this;
		string path = "";
		vector<string> v = split(fName, '/');
		if (fName[0] == '/')
			path = "/";
		if (idx != string::npos) {
			for (vector<string>::iterator it = v.begin(); it != v.end() - 1; ++it) path += (*it) + "/";
			xx = getByPath(path);
		}

		if (xx != NULL) {
			int position = 0;
			for (vector<File*>::iterator it = xx->files.begin(); it != xx->files.end(); ++it) {
				File *x = *it;
				if (idx != string::npos) {
					fName = v[v.size() - 1];
				}
				if (x->name == fName) {
					xx->files.erase(xx->files.begin() + position);
					found = true;
					break;
				}
				position++;
			}
		}
		if (!found)
			cout << "File does not exist" << endl;
	}
	void cat(string fName) {
		bool found = false;
		size_t idx = fName.find('/');
		Folder *xx = this;
		string path = "";
		vector<string> v = split(fName, '/');
		if (fName[0] == '/')
			path = "/";
		if (idx != string::npos) {
			for (vector<string>::iterator it = v.begin(); it != v.end() - 1; ++it) path += (*it) + "/";
			xx = getByPath(path);
		}

		if (xx != NULL) {
			for (vector<File*>::iterator it = xx->files.begin(); it != xx->files.end(); ++it) {
				File *x = *it;
				if (idx != string::npos) {
					fName = v[v.size() - 1];
				}
				if (x->name == fName) {
					cout << x->data << endl;
					found = true;
					break;
				}
			}
		}
		if (!found)
			cout << "File does not exist" << endl;
	}

	void cp(string fName, string dest) {
		bool found = false;
		if (checkExists(fName, this)) {
			File *temp;
			Folder *dFolder = getByPath(dest);
			if (dFolder == NULL) {
				cout << "Error: Invalid destination path" << endl;
			}
			else {
				if (!checkExistsBeforeCP(fName, dFolder)) {
					for (vector<File*>::iterator it = files.begin(); it != files.end(); ++it) {
						File *x = *it;
						if (x->name == fName) {
							temp = x;
							break;
						}
					}
					dFolder->files.push_back(temp);
					found = true;
				}

				else {
					found = true;
					cout << "Error: File with the same name already exists in destination folder" << endl;
				}
			}
		}
		if (!found)
			cout << "Error: File or destination folder does not exist" << endl;
	}
	bool isValidName(string fName) {
		bool valid = true;
		size_t idx = fName.find('/');
		if (idx != string::npos) {
			valid = false;
		}
		idx = fName.find('\\');
		if (idx != string::npos) {
			valid = false;
		}
		return valid;
	}
	void rnd(string path, string fName) {
		Folder *f = getByPath(path);
		if (isValidName(fName) && f != NULL && f->parent != NULL && !checkExists(fName, f->parent)) {
			f->name = fName;
		}
		else {
			cout << "Folder does not exist or a folder already exists with the same name" << endl;
		}
	}
	void rn(string p, string fName) {
		size_t idx = p.find('/');
		Folder *xx = this;
		string path = "";
		vector<string> v = split(p, '/');
		if (p[0] == '/')
			path = "/";
		if (idx != string::npos) {
			for (vector<string>::iterator it = v.begin(); it != v.end() - 1; ++it) path += (*it) + "/";
			xx = getByPath(path);
		}

		if (xx != NULL && isValidName(fName) && !checkExists(fName, xx)) {
			for (vector<File*>::iterator it = xx->files.begin(); it != xx->files.end(); ++it) {
				File *x = *it;
				if (idx != string::npos) {
					p = v[v.size() - 1];
				}
				if (x->name == p) {
					x->name = fName;
					break;
				}
			}
		}
		else if (!isValidName(fName))
			cout << "File name can't have back or forward slashes" << endl;
		else
			cout << "Error: File doesn't exist or file or folder already exists with the same name" << endl;

	}
	void mv(string fName, string dest) {
		bool found = false;
		if (checkExists(fName, this)) {
			File *temp;
			Folder *dFolder = getByPath(dest);
			if (dFolder == NULL) {
				cout << "Error: Invalid destination path" << endl;
			}
			else if (!checkExistsBeforeCP(fName, dFolder)) {
				for (vector<File*>::iterator it = files.begin(); it != files.end(); ++it) {
					File *x = *it;
					if (x->name == fName) {
						temp = x;
						break;
					}
				}
				dFolder->files.push_back(temp);
				found = true;
			}
			else {
				cout << "Error: File with the same name already exists in destination folder" << endl;
			}
		}
		if (!found)
			cout << "Error: File or destination folder does not exist" << endl;
		else
			rm(fName);
	}

	void pwd(Folder *x, string &res) {
		if (x != NULL) {
			pwd(x->parent, res);
			res += x->name + "/";
		}
		else {
			res = "";
		}
	}

	Folder *getRoot(Folder *x) {
		if (x->parent == NULL) {
			return x;
		}
		else
			return getRoot(x->parent);
	}

	Folder *getByPath(string path) {
		vector<string> v = split(path, '/');
		Folder *root = this;
		if (path == "/") {
			return getRoot(this);
		}
		if (path[0] == '/') {
			root = getRoot(this);
		}
		int p = 0;
		for (vector<string>::const_iterator i = v.begin(); i != v.end(); ++i) {
			p++;
			for (vector<Folder*>::iterator it = root->children.begin(); it != root->children.end(); ++it) {
				Folder *x = *it;
				if (x->name == *i && p == v.size()) {
					return x;
				}
				else if (x->name == *i) {
					root = x;
					break;
				}
			}
		}
		return NULL;
	}
	void createRealFiles(Folder *f, string path) {
		string fullpath;
		for (vector<File*>::iterator it = f->files.begin(); it != f->files.end(); ++it) {
			File *x = *it;
			fullpath = path + "\\" + f->name + "\\" + x->name;
			FILE *fp;
			fp = fopen(fullpath.c_str(), "wb");
			if (fp == NULL) {
				cout << fullpath << endl;
				perror("error");
			}
			else {
				fwrite(x->data, 1, x->size, fp);
				fclose(fp);
			}
		}
	}
	void createRealDir(Folder *f, string path, string fpath, bool isChild) {
		string fullpath = "";
		for (vector<Folder*>::iterator it = f->children.begin(); it != f->children.end(); ++it) {
			Folder *x = *it;
			fullpath = path + fpath + "\\" + x->name;
			CreateDirectoryA(fullpath.c_str(), NULL);
			createRealFiles(x, path + fpath + "\\");
			createRealDir(x, path, fpath + "\\" + x->name, true);
		}
	}
	void dumpall(string path) {
		string pwd;
		this->pwd(this, pwd);
		string fullpath = path + pwd;
		const char *x = path.c_str();
		vector<string> folders = split(pwd, '/');
		struct stat info;
		if (stat(x, &info) != 0)
			cout << "cannot access " << x << endl;
		else if (info.st_mode & S_IFDIR) {
			Folder *f = getByPath(pwd);
			createRealFiles(f, path);
			createRealDir(f, path, "\\", false);
		}
		else {
			cout << "specified path does not exist " << endl;
		}


	}
	void dumpFromDisk(string path, Folder *f, string gp = "/") {
		struct stat s;
		for (const auto & entry : fs::directory_iterator(path)) {
			string p = ((entry.path()).u8string());
			vector<string> pv = split(p, '\\');
			string fName = pv[pv.size() - 1];
			string pp = "";
			string pp2 = "";
			for (vector<string>::iterator it = pv.begin(); it != pv.end() - 1; ++it) pp += (*it) + "\\";
			if (stat(p.c_str(), &s) == 0)
			{
				if (s.st_mode & S_IFDIR)
				{
					cout << "Creating folder: " << gp + fName << endl;
					f->add_child(fName);
					Folder *f = getByPath(gp + fName);
					if (f != NULL) {
						dumpFromDisk(path + "\\" + fName, f, gp + fName + "/");
					}
				}
				else if (s.st_mode & S_IFREG)
				{
					cout << "Creating file: " << gp + fName << endl;
					f->add_ext_file(fName, p);
				}
			}
		}
	}

};


void menu() {
	Folder *current = new Folder();
	current->name = "";
	char input[1000];
	string command;
	cout << "Type 'help' to display commands list " << endl;
	bool run = true;
	string dir;
	while (run) {
		current->pwd(current, dir);
		cout << "[" << dir << "] $:";

		cin.getline(input, sizeof(input));
		command = input;
		vector<string> c = split(command, ' ');
		if (c.size() < 1)
			c.push_back(" ");
		if (c[0] == "help") {
			cout << "usage: [command] [args]" << endl
				<< "ls                : List foldres in current directory" << endl
				<< "cd dir	    	  : Change current directory. Ex: cd /foo/bar" << endl
				<< "mkdir dirname	  : Create directory. Ex: mkdir foo" << endl
				<< "rd dirname        : Remove directory. Ex: rd foo" << endl
				<< "rm filename 	  : Remove File: Ex: rm foo.txt" << endl
				<< "cfe filename path : Create file from disk. Ex: cfe d:\\foo.txt" << endl
				<< "cp filename dest  : Copy file to another folder. Ex: cp foo.txt" << endl
				<< "mv filename dest  : Move file to another folder. Ex: mv foo.txt" << endl
				<< "rn filename       : Rename file" << endl
				<< "rnd dirname       : Rename folder" << endl
				<< "cat filename      : Read file. Ex: cat foo.txt" << endl
				<< "dump path         : Write folders and files in current directory to your disk. Ex: dump d:\\test" << endl
				<< "import path       : Import folders and files recursively from your disk to current directory. Ex: import d:\\test" << endl
				<< "pwd               : Print current directory" << endl
				<< "exit              : Exit" << endl;
		}
		else if (c[0] == "ls") {
			current->ls();
		}
		else if (c[0] == "cd") {
			if (c.size() > 1) {
				string dir = c[1];
				current = current->cd(dir);
			}
		}
		else if (c[0] == "mkdir") {
			if (c.size() > 1) {
				string dir = c[1];
				current->add_child(dir);
			}
		}
		else if (c[0] == "cf") {
			if (c.size() > 1) {
				string name = c[1];
				cout << "Enter file contents:" << endl;
				cin.getline(input, sizeof(input));
				string data = input;
				current->add_file(name, data);
			}
		}
		else if (c[0] == "rd") {
			if (c.size() > 1) {
				string name = c[1];
				current->rd(name);
			}
		}
		else if (c[0] == "rm") {
			if (c.size() > 1) {
				string name = c[1];
				current->rm(name);
			}
		}
		else if (c[0] == "cfe") {
			if (c.size() > 1) {
				string name = c[1];
				cout << "Enter file path: ";
				cin.getline(input, sizeof(input));
				string path = input;
				current->add_ext_file(name, path);
			}
		}

		else if (c[0] == "cat") {
			if (c.size() > 1) {
				string name = c[1];
				current->cat(name);
			}
		}
		else if (c[0] == "cp") {
			if (c.size() > 1) {
				string name = c[1];
				cout << "Enter destination folder path: ";
				cin.getline(input, sizeof(input));
				string dest = input;
				current->cp(name, dest);
			}
		}

		else if (c[0] == "mv") {
			if (c.size() > 1) {
				string name = c[1];
				cout << "Enter destination folder path: ";
				cin.getline(input, sizeof(input));
				string dest = input;
				current->mv(name, dest);
			}
		}

		else if (c[0] == "pwd") {
			string pwd = "";
			current->pwd(current, pwd);
			cout << pwd << endl;
		}

		else if (c[0] == "exit") {
			run = false;
		}
		else if (c[0] == "dump") {
			if (c.size() > 1) {
				string path = c[1];
				current->dumpall(path);
			}
		}
		else if (c[0] == "rnd") {
			if (c.size() > 1) {
				string path = c[1];
				cout << "Enter new folder name: ";
				cin.getline(input, sizeof(input));
				string fName = input;
				current->rnd(path, fName);
			}
		}
		else if (c[0] == "rn") {
			if (c.size() > 1) {
				string path = c[1];
				cout << "Enter new folder name: ";
				cin.getline(input, sizeof(input));
				string fName = input;
				current->rn(path, fName);


			}
		}
		else if (c[0] == "import") {
			if (c.size() > 1) {
				string path = c[1];
				current->dumpFromDisk(path, current);
			}
		}
		else if (c[0] != " ") {
			cout << "Invalid Command" << endl;
		}

	}

}

int main()
{
	menu();
	return 0;
}

