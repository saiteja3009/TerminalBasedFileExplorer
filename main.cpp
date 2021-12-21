#include <bits/stdc++.h>
#include <unistd.h>
#include <filesystem>
#include <iostream>
#include <string>
#include <dirent.h>
#include <sys/stat.h>
#include <termios.h>
#include <stdlib.h>
#include <grp.h>
#include <pwd.h>
#include <ctime>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#define MAX 5
using namespace std;

int top = 0;
char *cwd;
int bottom = MAX;
int col;
char *home;
int statusline = MAX + 3;
struct winsize w_size;
int x = 1, y = 1;
vector<string> files;
struct termios term;
stack<string> lstack;
stack<string> rstack;
vector<string> commands;
int comcur = 11;

void cursor(int x, int y)
{
	cout << "\033[" << x << ";" << y << "H";
}
void clearline()
{
	cout << "\033[K";

}

void printstatus(string mes)
{

	cursor(statusline, 1);
	clearline();
	cout << mes;

}

void normal()
{
	tcgetattr(STDIN_FILENO, &term);
	struct termios cur = term;
	cur.c_lflag &= ~(ECHO | ICANON);
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &cur) != 0)
	{
		printstatus("Unable to Switch to Normal Mode");
		return;
	}
}

void denormal()
{
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &term);
}
void reset()
{
	x = 1;
	y = 0;
}

void clear()
{
	cout << "\033[H\033[2J\033[3J";
}
void print(const char *list)
{
	struct stat t;
	struct passwd * pass;
	struct group * gro;
	string p;
	string d = string(cwd);
	string c = string(list);
	p = d + "/" + c;
	char *path = new char[p.length() + 1];
	strcpy(path, p.c_str());
	if (stat(path, &t) == -1)
	{
		printstatus("ERROR");
		return;
	}
	printf((S_ISDIR(t.st_mode)) ? "d" : "-");
	cout << ((t.st_mode &S_IRUSR) ? "r" : "-");
	cout << ((t.st_mode &S_IWUSR) ? "w" : "-");
	cout << ((t.st_mode &S_IXUSR) ? "x" : "-");
	cout << ((t.st_mode &S_IRGRP) ? "r" : "-");
	cout << ((t.st_mode &S_IWGRP) ? "w" : "-");
	cout << ((t.st_mode &S_IXGRP) ? "x" : "-");
	cout << ((t.st_mode &S_IROTH) ? "r" : "-");
	cout << ((t.st_mode &S_IWOTH) ? "w" : "-");
	cout << "  ";
	pass = getpwuid(t.st_uid);
	printf("%5s ", pass->pw_name);
	cout << "    ";
	gro = getgrgid(t.st_gid);
	printf("%5s ", gro->gr_name);
	long long int s = t.st_size;
	cout << "    ";
	char *time = (ctime(&t.st_mtime));
	time[strlen(time) - 1] = '\0';
	string ti = string(time);
	cout << (ti.substr(ti.length() - 20));
	cout << " ";
	if ((s / (1 << 20)) > 1)
		cout << s / (1 << 20) << "M";
	else if ((s / (1 << 10)) > 1)
		cout << s / (1 << 10) << "K";
	else
		cout << s << "B";
	if (S_ISDIR(t.st_mode))
	{
		if (strlen(list) <= 10)
			cout << "   \033[1;32m" << list<< "\033[0m";
		else
		{
			string x = string(list);
			cout << "   \033[1;32m" << x.substr(0, 10) << "..." << "\033[0m";
		}
	}
	else
	{
		if (strlen(list) <= 10)
			cout << "   \033[1;32m" << list<< "\033[0m";
		else
		{
			string x = string(list);
			cout << "   \033[1;36m" << x.substr(0, 10) << "..." << "\033[0m";
		}
	}
	cout << "\n";
}
void reset2()
{
	top = 0;
	bottom = MAX;
}

void up()
{
	//if(ch==65){
	if (x - 1 > 0)
	{
		cursor(--x, 1);
	}

	//}
}
void down()
{
	if (x + 1 <= min(bottom, MAX))
	{
		cursor(++x, 1);
	}
}

void display(int a, int b)
{
	int i = a;
	while (i < b)
	{
		print(files[i].c_str());
		i++;
	}
	reset();
	printstatus("Normal Mode");
	cursor(x, 1);

}

void getall(char *cwd)
{
	DIR * dir;
	dir = opendir(cwd);
	if (dir == NULL)
		printstatus("Empty");
	chdir(cwd);
	files.clear();
	struct dirent * ent;
	while ((ent = readdir(dir)) != NULL)
	{
		if (strlen(ent->d_name) == 1 and ent->d_name[0] == '.') files.push_back(ent->d_name);
		else if (strlen(ent->d_name) == 2 and ent->d_name[1] == '.') files.push_back(ent->d_name);
		else if (ent->d_name[0] != '.') files.push_back(ent->d_name);
	}

	closedir(dir);
	sort(files.begin(), files.end());
	clear();
	top = 0;
	bottom = files.size();
	bottom = min(bottom, MAX);
	display(top, min(bottom, MAX));
}
void process(char *cd)
{

	getall(cd);
}
void goback()
{
	clear();
	//cout<<cwd<<home;
	if (cwd == home)
	{
		return;
	}
	lstack.push(string(cwd));
	rstack.push(string(cwd));
	string path = string(cwd);
	int o;
	for (int i = 0; i < path.length(); i++)
	{
		if (path[i] == '/')
			o = i;
	}
	string ppath = path.substr(0, o);
	strcpy(cwd, ppath.c_str());
	process(cwd);
	return;
}

void enter()
{

	struct stat t;
	string present = files[top + x - 1];
	//process((string(cwd) + "/" + present).c_str());
	//return;
	stat(present.c_str(), &t);
	if (S_ISDIR(t.st_mode))
	{
		if (present.compare(".") == 0)
			return;
		if (present.compare("..") == 0)
		{
			//clear();
			goback();
			//cout<<cwd;
			return;
		}
		string fullpath = string(cwd) + "/" + present;
		char *fp = new char[fullpath.length() + 1];
		strcpy(fp, fullpath.c_str());
		lstack.push(string(cwd));
		strcpy(cwd, fp);
		cout << present;
		process(cwd);
	}
	else
	{
		pid_t pid = fork();
		if (pid == 0)
		{
			execl("/usr/bin/xdg-open", "xdg-open", present.c_str(), NULL);
			exit(1);
		}
	}

	return;
}

void upk()
{
	if (x != 1 || top == 0)
		return;
	clear();
	display(--top, --bottom);
}

void downl()
{
	if (x != min(MAX, bottom) || bottom >= files.size())
		return;
	clear();
	display(++top, ++bottom);
	x = bottom - top;
	cursor(x, 1);
}
void gohome()
{
	if (cwd == home)
	{
		return;
	}
	while (!lstack.empty())
		lstack.pop();
	while (!rstack.empty())
		rstack.pop();
	strcpy(cwd, home);
	process(cwd);
	return;
}

void goprev()
{
	if (lstack.empty())
		return;
	string toppath = lstack.top();
	lstack.pop();
	rstack.push(cwd);
	strcpy(cwd, toppath.c_str());
	process(cwd);
	return;
}
void gofor()
{
	if (rstack.empty())
		return;
	string toppath = rstack.top();
	rstack.pop();
	lstack.push(cwd);
	strcpy(cwd, toppath.c_str());
	process(cwd);
	return;
}
void processcwd(char *current)
{
	strcpy(cwd, current);
	process(cwd);
	cursor(statusline, 1);
	clearline();
	cout << "Command Mode";
}

void copyfile(string src, string dest)
{
	char c;
	FILE *from, *to;
	from = fopen(src.c_str(), "r");
	to = fopen(dest.c_str(), "w");
	if (from == NULL || to == NULL)
	{
		printstatus("No file/ dir exists");
		return;
	}
	while ((c = getc(from)) != EOF)
		putc(c, to);
	struct stat txt;
	stat(src.c_str(), &txt);
	chown(dest.c_str(), txt.st_uid, txt.st_gid);
	chmod(dest.c_str(), txt.st_mode);
	fclose(to);
	fclose(from);
	return;
}

void copydir(string src, string dest)
{

	DIR * direct;
	struct dirent * d;
	struct stat info;
	direct = opendir(src.c_str());
	clear();
	if (direct == NULL)
		{printstatus("Wrong dir/file opened");
		return;}
	clear();
	while ((d = readdir(direct)))
	{
		string sname = src + "/" + string(d->d_name);
		string dname = dest + "/" + string(d->d_name);
		string x=string(d->d_name);
		stat(sname.c_str(), &info);
		string cname = string(d->d_name);
		if (cname == "." || cname == "..")
			continue;
		else if (S_ISDIR(info.st_mode))
		{
		
		mkdir(dname.c_str(),0777);
		clear();
			copydir(sname, dname);
		}
		else
		{//mkdir(dname.c_str(),0777);

			copyfile(sname, dname);
			}
	}
	closedir(direct);
}

void copyfunc()
{	clear();
	string dest = commands[commands.size() - 1];
	if (dest[0] == '.' || dest[0] == '~')
	{
		string curdest = string(home) + dest.substr(1, dest.length());
		for (int i = 1; i < commands.size() - 1; i++)
		{
			string src = "";
			if (commands[i][0] == '.' || commands[i][0] == '~')
				{src = string(home) + commands[i].substr(1, commands[i].length());
				}
			else
				src = commands[i];
			int op = 0;
			for (int j = 0; j < src.length(); j++)
			{
				if (src[j] == '/')
					op = j;
			}
			string total = curdest + "/" + src.substr(op + 1, src.length() - op);
			clear();
			printstatus(src+" "+total);
			struct stat txt;
			stat(src.c_str(), &txt);

			if (S_ISDIR(txt.st_mode))
			{mkdir(total.c_str(),0777);
				copydir(src, total);
				}
			else
				copyfile(src, total);
		}
	}
	else
	{
		for (int i = 1; i < commands.size() - 1; i++)
		{
			string src = "";
			if (commands[i][0] == '.' || commands[i][0] == '~')
				src = string(home) + commands[i].substr(1, commands[i].length());
			else
				src = commands[i];
			int op = 0;
			for (int j = 0; j < src.length(); j++)
			{
				if (src[j] == '/')
					op = j;
			}
			string total = dest + "/" + src.substr(op + 1, src.length() - op);
			struct stat txt;
			stat(src.c_str(), &txt);
			if (S_ISDIR(txt.st_mode))
				{mkdir(total.c_str(),0777);
				copydir(src, total);
				}
			else
				copyfile(src, total);
		}
	}
}
void delete_file(string name)
{
	string res = "";
	if (name[0] == '.' || name[0] == '~')
	{
		res = string(home) + name.substr(1, name.length());
	}
	else
		res = name;
	int x = remove(res.c_str());
	if (x != 0)
	{
		printstatus("Error");
	}
}

void delete_dir(string src)
{
	string res = "";
	if (src[0] == '.' || src[0] == '~')
	{
		res = string(home) + src.substr(1, src.length());
	}
	else
		res = src;

	DIR * direct;
	struct dirent * d;
	struct stat info;
	direct = opendir(res.c_str());
	if (direct == NULL)
		{printstatus("Error");
		return;
		}
	while ((d = readdir(direct)))
	{
		string sname = res + "/" + string(d->d_name);
		string cname = string(d->d_name);
		stat(sname.c_str(), &info);
		if (cname == "." || cname == "..")
			continue;
		else if (S_ISDIR(info.st_mode))
		{
			delete_dir(sname);
		}
		else
			delete_file(sname);
	}
	closedir(direct);
	remove(res.c_str());
}

void movefunc()
{

	string dest = commands[commands.size() - 1];
	if (dest[0] == '.' || dest[0] == '~')
	{
		string curdest = string(home) + dest.substr(1, dest.length());
		for (int i = 1; i < commands.size() - 1; i++)
		{
			string src = "";
			if (commands[i][0] == '.' || commands[i][0] == '~')
				src = string(home) + commands[i].substr(1, commands[i].length());
			else
				src = commands[i];
			int op = 0;
			for (int j = 0; j < src.length(); j++)
			{
				if (src[j] == '/')
					op = j;
			}
			string total = curdest + "/" + src.substr(op + 1, src.length() - op);
			struct stat txt;
			stat(src.c_str(), &txt);

			if (S_ISDIR(txt.st_mode))
			{
				mkdir(total.c_str(),0777);
				copydir(src, total);
				delete_dir(src);
			}
			else
			{
				copyfile(src, total);
				delete_file(src);
			}
		}
	}
	else
	{
		for (int i = 1; i < commands.size() - 1; i++)
		{
			string src = "";
			if (commands[i][0] == '.' || commands[i][0] == '~')
				src = string(home) + commands[i].substr(1, commands[i].length());
			else
				src = commands[i];
			int op = 0;
			for (int j = 0; j < src.length(); j++)
			{
				if (src[j] == '/')
					op = j;
			}
			string total = dest + "/" + src.substr(op + 1, src.length() - op);
			struct stat txt;
			stat(src.c_str(), &txt);
			if (S_ISDIR(txt.st_mode))
				{
				mkdir(total.c_str(),0777);
				copydir(src, total);
				delete_dir(src);
				}
			else
				{
				copyfile(src, total);
				delete_file(src);
			}
		}
	}
}

void create_file()
{
	string dest = commands[commands.size() - 1];
	for (int i = 1; i < commands.size() - 1; i++)
	{
		if (dest[0] == '.' || dest[0] == '~')
		{
			string curdest = dest.substr(1, dest.length());
			open((home + curdest + '/' + commands[i]).c_str(), O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
		}
		else
		{
			open((dest + '/' + commands[i]).c_str(), O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
		}
	}
}

void create_dir()
{
	string dest = commands[commands.size() - 1];
	for (int i = 1; i < commands.size() - 1; i++)
	{
		if (dest[0] == '.' || dest[0] == '~')
		{
			string curdest = dest.substr(1, dest.length());
			mkdir((home + curdest + '/' + commands[i]).c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		}
		else
		{
			mkdir((dest + '/' + commands[i]).c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		}
	}
}

bool search(string current, string key)
{
	DIR * direct;
	struct dirent * d;
	struct stat info;
	direct = opendir(current.c_str());
	if (direct == NULL)
		return false;
	while ((d = readdir(direct)))
	{
		stat(d->d_name, &info);
		string cname = string(d->d_name);
		if (cname == key)
			return true;
		string next = current + "/" + cname;
		if (cname == "." || cname == "..")
			continue;
		else if (S_ISDIR(info.st_mode))
		{
			bool op = search(next, key);
			if (op == true) return true;
		}
	}
	closedir(direct);
	return false;

}

void execute()
{
	if (commands[0] == "copy")
	{
		copyfunc();
		processcwd(cwd);
		cursor(statusline + 2, 1);
		clearline();
		cout << "Copied Successfully";
	}
	else if (commands[0] == "move")
	{

		movefunc();
		processcwd(cwd);
		cursor(statusline + 2, 1);
		clearline();
		cout << "Moved Successfully";
	}
	else if (commands[0] == "rename")
	{
	string res,res1;
		if(commands[1][0]=='.' || commands[1][0]=='~')
			res=home+commands[1].substr(1,commands[1].length());
			else
			res=commands[1];
		if(commands[2][0]=='.'||commands[2][0]=='~')
		{
			res1=home+commands[2].substr(1,commands[2].length());
			}
			else
			res1=commands[2];
		rename(res.c_str(), res1.c_str());
		processcwd(cwd);
		cursor(statusline + 2, 1);
		clearline();
		cout << "Renamed Successfully";
	}
	else if (commands[0] == "goto")
	{
		if (commands[1][0] != '.' && commands[1][0] != '~')
			strcpy(cwd, commands[1].c_str());
		else
			strcpy(cwd, (home + commands[1].substr(1, commands[1].length())).c_str());
		processcwd(cwd);
		cursor(statusline + 2, 1);
		clearline();
		cout << "Path Changed Successfully";
	}
	else if (commands[0] == "create_file")
	{
		create_file();
		processcwd(cwd);
		cursor(statusline + 2, 1);
		clearline();
		cout << "Created File Successfully";
	}
	else if (commands[0] == "create_dir")
	{
		create_dir();
		processcwd(cwd);
		cursor(statusline + 2, 1);
		clearline();
		cout << "Created Directory Successfully";
	}
	else if (commands[0] == "search")
	{
		bool ans = search(string(cwd), commands[1]);
		cursor(statusline + 2, 1);
		clearline();
		if (ans == true)
			cout << "Found Successfully";
		else
			cout << "Not Found";
	}
	else if (commands[0] == "delete_file")
	{
		delete_file(commands[1]);
		processcwd(cwd);
		cursor(statusline + 2, 1);
		clearline();
		cout << "Deleted File Successfully";
	}
	else if (commands[0] == "delete_dir")
	{
		delete_dir(commands[1]);
		processcwd(cwd);
		cursor(statusline + 2, 1);
		clearline();
		cout << "Deleted Directory Successfully";
	}
	else
	{
		cursor(statusline + 2, 1);
		clearline();
		cout << "Invalid Command";
	}
}

void command()
{
	printstatus("Command Mode");
	cursor(statusline + 1, 1);
	int op = 1;
	//while(true){
	char ch;
	commands.clear();
	bool flag = true;
	string key = "";
	while (true)
	{
		ch = cin.get();
		if (ch == 27)
		{
			clearline();
			cursor(statusline, 1);
			printstatus("Normal Mode");
			cursor(statusline + 1, 1);
			clearline();
			cursor(statusline + 2, 1);
			clearline();
			cursor(x, 1);
			return;
		}
		else if (ch == ' ')
		{
			commands.push_back(key);
			flag = false;
			key = "";
		}
		else if (ch == '\n')
		{
			commands.push_back(key);
			execute();
			cursor(statusline + 1, 1);
			clearline();
			commands.clear();
			flag = true;
			key = "";
			op = 0;
		}
		else
			key = key + ch;
		if (ch != '\n')
			cout << ch;
		cursor(statusline + 1, ++op);
	}
}
int main()
{

	cwd = get_current_dir_name();
	home = get_current_dir_name();
	process(cwd);
	char ch;
	normal();
	while (true)
	{
		ch = cin.get();
		if (ch == 'q')
		{
			break;
		}
		switch (ch)
		{
			case 65:
				up();
				break;
			case 66:
				down();
				break;
			case 107:
				upk();
				break;
			case 108:
				downl();
				break;
			case 10:
				enter();
				break;
			case 104:
				gohome();
				break;
			case 127:
				goback();
				break;
			case 68:
				goprev();
				break;
			case 67:
				gofor();
				break;
			case ':':
				command();
				break;
			default:
				break;
		}
	}
	atexit(denormal);

	return 0;
}
