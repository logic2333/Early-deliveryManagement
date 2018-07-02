/***********************
本程序由我一人完成
主要功能：从package.txt和poster.txt中读入包裹和快递员的信息，进行增删查改后保存
可安排空闲的快递员去运送包裹
主要运用了vector容器和字符串流
在VS2013上测试通过，代码规模：300行
***********************/

#include <iostream>
#include <conio.h>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
using namespace std;

//包裹的六种状态
const string status_info[6] = { "还未发货", "在路上", "待配送", "配送中", "已签收", "已丢失" };
typedef enum { wait_sent, on_way, wait_distr, distr, sent, lost } status_type;
//三个类的声明（struct与class的区别，仅在于struct中的成员默认全为public而class全为private，因此对于所有成员均为public的类，选用struct）
class poster_type; 
struct package_type;
struct manage;
//vector可以很方便地实现增删查改
vector<package_type> packages_for_sent;		
vector<poster_type> posters;

struct person_type {
	string name, tel, addr;
};

class package_type {
	friend class manage;
	person_type sender, receiver;
	unsigned weight, fare;
	status_type status;
public:
	string ID;
	package_type() {}
	package_type(const string& line) {		//line为package.txt中的一行字符串
		if (line.back() < '4') {			//对于“已签收”及“已丢失”的包裹，不予录入
			istringstream temp(line);		//字符串流处理，将一行含有空格的字符串自然拆散读入
			temp >> receiver.name >> receiver.tel >> receiver.addr
				>> sender.name >> sender.tel >> sender.addr
				>> ID >> weight >> fare;
			unsigned t; temp >> t;
			status = status_type(t);
		}
	}
	void print() const {
		cout << "收件人姓名：" << receiver.name << "  收件人电话：" << receiver.tel << "  收件人地址：" << receiver.addr << endl;
		cout << "发件人姓名：" << sender.name << "  发件人电话：" << sender.tel << "  发件人地址：" << sender.addr << endl;	
		cout << "包裹重量：" << weight << "  运费：" << fare << endl;
		cout << "包裹编号：" << ID << "  包裹状态：" << status_info[status] << endl;
	}
	string print_file() const {				//保存时将每一个包裹的信息存放为与读入时格式一致的一个字符串，同样采用流处理
		ostringstream os;
		os << receiver.name << ' ' << receiver.tel << ' ' << receiver.addr << ' '
			<< sender.name << ' ' << sender.tel << ' ' << sender.addr << ' '
			<< ID << ' ' << weight << ' ' << fare << ' ' << unsigned(status);
		return os.str();
	}
};

class poster_type {
	friend class manage;
	string name;
	vector<package_type>::iterator duty;		//某一位快递员负责的一个包裹，STL容器的迭代器相当于数组的指针
public:
	bool available;								//该快递员是否空闲
	poster_type() {}
	poster_type(const string& line) {			//line为poster.txt中的一行字符串
		if (find(line.begin(), line.end(), ' ') == line.end()) {	//如果读入的字符串中不含空格（只有一个人名），表示该快递员空闲
			available = true; name = line; duty = packages_for_sent.end();
		}
		else {									//否则，人名后面为该快递员负责包裹的编号
			istringstream temp(line);			//用字符串流将line断开
			temp >> name;
			available = false;
			string id_str; temp >> id_str;
			for (auto i = packages_for_sent.begin(); i != packages_for_sent.end(); i++)
				if (i->ID == id_str) {			//找到这个编号的包裹后，将其迭代器赋给duty
					duty = i; break;
				}
		}
	}
	void print() const {
		cout << name << ' ';
		if (available) cout << "空闲" << endl;
		else cout << "负责包裹：" << duty->ID << endl;
	}
	string print_file() const {					//保存时将每一个包裹的信息存放为与读入时格式一致的一个字符串
		if (available) return name;
		else return name + ' ' + duty->ID;
	}
};

struct manage {
	void add_package(const string& line) {
		packages_for_sent.emplace_back(line);	//emplace为C++11新特性，在STL容器的一个特定位置原位调用对象的构造函数，减少了一次拷贝，下同
	}
	vector<package_type>::iterator search_package_ID(const string& id_str) {
		auto i = packages_for_sent.begin();		//auto为C++11新特性，可自动推导数据类型，避免输入冗长的vector<package_type>::iterator，以下类似
		for (; i != packages_for_sent.end() && i->ID != id_str; i++);
		return i;								//如果没找到返回的是容器的end()迭代器，指向容器最后的空位置，下同
	}
	vector<package_type>::iterator search_package_sender(const string& sender_name) {
		auto i = packages_for_sent.begin();
		for (; i != packages_for_sent.end() && i->sender.name != sender_name; i++);
		return i;								
	}
	vector<package_type>::iterator search_package_receiver(const string& receiver_name) {
		auto i = packages_for_sent.begin();
		for (; i != packages_for_sent.end() && i->sender.name != receiver_name; i++);
		return i;								
	}
	vector<poster_type>::iterator search_poster(const string& poster_name) {
		auto i = posters.begin();
		for (; i != posters.end() && i->name != poster_name; i++);
		return i;								
	}
	bool delete_package(const string& id_str) {
		auto delete_posit = search_package_ID(id_str);
		if (delete_posit == packages_for_sent.end()) return false;	//没找到
		else {
			packages_for_sent.erase(delete_posit); return true;
		}
	}
	void add_poster(const string& ipt) {
		posters.emplace_back(ipt);
	}
	bool assign_package(const string& ipt) {		//为一个待运送的包裹指定一名空闲的快递员去运送，为保持接口的一致性参数同样设为一个输入的字符串
		string package_id, poster_name;
		istringstream is(ipt);
		is >> package_id >> poster_name;			//将输入拆成一个包裹的编号和快递员的姓名
		auto i = search_package_ID(package_id);
		auto j = search_poster(poster_name);
		if (j != posters.end() && j->available		//找到了该快递员且他是空闲的
			&& i != packages_for_sent.end() &&		//找到了包裹且包裹状态为“还未发货”或“待配送”
			(i->status == wait_sent || i->status == wait_distr)) {
			i->status = status_type(i->status + 1); //更改包裹状态为“在路上”或“配送中”
			j->duty = i;								//更改快递员的任务为这个包裹
			j->available = false;
			return true;
		}
		else return false;
	}
	bool delete_poster(const string& poster_name) {
		auto delete_posit = search_poster(poster_name);
		if (delete_posit >= posters.begin() && delete_posit < posters.end() && delete_posit->available) {
			//找到了该快递员且该快递员处于空闲状态才能删除（辞退）
			posters.erase(delete_posit);
			return true;
		}
		else return false;
	}
};

void init() {
	cout << "******************************" << endl;
	cout << "******快递管家 By Logic*******" << endl;
	cout << "******************************" << endl;
	cout << "从package.txt读入包裹信息（已签收或丢失的包裹将不会被读入）...";
	ifstream file("package.txt");
	while (!file.eof()) {
		string ipt; getline(file, ipt);				//getline函数从流中读入完整的一行，下同
		packages_for_sent.emplace_back(ipt);
	}
	file.close();
	cout << "读取完毕！" << endl;
	cout << "从poster.txt读入快递员信息...";
	file.open("poster.txt");
	while (!file.eof()) {
		string ipt; getline(file, ipt);
		posters.emplace_back(ipt);
	}
	file.close();
	cout << "读取完毕！" << endl;
}

void save() {
	ofstream file("poster.txt", ios::trunc);		//将源文件全部重写（对于小规模的数据，简单易行），下同
	for (auto poster : posters) file << poster.print_file() << endl;	//C++11提供的新的遍历方式，类似于VB和C#中的for each语句，下同
	file.close();
	file.open("package.txt", ios::trunc);
	for (auto package : packages_for_sent) file << package.print_file() << endl;
	file.close();
}

int main()
{
	init();
	manage mn;
	bool ex = false;		//当ex为true时退出程序
	do {
		cout << endl << "请输入您的操作：1 - 查看所有快递员信息  2 - 查看所有包裹信息  3 - 新增快递员  4 - 新增包裹  5 - 查找快递员  6 - 查找包裹  "
			<< "7 - 删除快递员  8 - 删除包裹  9 - 为空闲快递员分配任务  其他 - 保存更改并退出";	//第一个输出的endl是为了使控制台看起来更清晰
		char op = _getche(); cout << endl;		//getche函数（在VS2013及以上的版本中须写作_getche）包含在conio.h中，实时监测键盘上按下的字符而不需要用户按下回车键，利于人机交互
		//主要功能在manage类中已实现，main函数中主要输出一些提示语，就不再注释了
		switch (op) {
			case '1': {
				for (auto poster : posters) poster.print();
				break;
			}
			case '2':{
				for (auto package : packages_for_sent) package.print();
				break;
			}
			case '3':{
				cout << "请输入快递员姓名：";
				string ipt; getline(cin, ipt);
				posters.emplace_back(ipt);
				cout << "新增一名空闲的快递员成功！" << endl;
				break;
			}
			case '4':{
				cout << "在下一行输入包裹信息，格式为：收件人姓名 电话 地址 发件人姓名 电话 地址 包裹编号 重量 运费 状态"
					<< "（0 - 还未发货  1 - 在路上  2 - 待配送  3 - 配送中  4 - 已签收  5 - 已丢失）" << endl;
				string ipt; getline(cin, ipt);
				packages_for_sent.emplace_back(ipt);
				cout << "新增一件包裹成功！" << endl;
				break;
			}
			case '5':{
				cout << "请输入快递员姓名：";
				string name; cin >> name;
				auto i = mn.search_poster(name);
				if (i == posters.end()) cout << "查无此人！" << endl;
				else i->print();
				break;
			}
			case '6':{
				cout << "您想按什么字段查找？ 0 - 发件人姓名  1 - 收件人姓名  2 - 包裹编号  其他 - 返回上一层";
				char t = _getche(); cout << endl;
				switch (t) {
					case '0': {
						cout << "请输入发件人姓名：";
						string name; cin >> name;
						auto i = mn.search_package_sender(name);
						if (i == packages_for_sent.end()) cout << "找不到此包裹！" << endl;
						else i->print();
						break;
					}
					case '1': {
						cout << "请输入收件人姓名：";
						string name; cin >> name;
						auto i = mn.search_package_receiver(name);
						if (i == packages_for_sent.end()) cout << "找不到此包裹！" << endl;
						else i->print();
						break;
					}
					case '2': {
						cout << "请输入包裹编号：";
						string id; cin >> id;
						auto i = mn.search_package_ID(id);
						if (i == packages_for_sent.end()) cout << "找不到此包裹！" << endl;
						else i->print();
						break;
					}
					default: break;
				}
				break;
			}
			case '7': {
				cout << "请输入要删除快递员的姓名：";
				string name; cin >> name;
				if (mn.delete_poster(name)) cout << "删除成功！" << endl;
				else cout << "没有找到该快递员，或该快递员正在工作中，无法删除！" << endl;
				break;
			}
			case '8': {
				cout << "请输入要删除包裹的编号：";
				string id; cin >> id;
				if (mn.delete_package(id)) cout << "删除成功！" << endl;
				else cout << "没有找到该包裹，无法删除！" << endl;
				break;
			}
			case '9': {
				cout << "请输入包裹编号 运送它的快递员姓名：";
				string ipt; getline(cin, ipt);
				if (mn.assign_package(ipt)) cout << "指派成功！" << endl;
				else cout << "该包裹不存在或已被运送，或该快递员不存在或正在工作中，指派失败！" << endl;
				break;
			}
			default: {
				cout << "确认要保存并退出吗？按Y或y退出，按其他键继续...";
				char t = _getche(); cout << endl;
				if (t == 'Y' || t == 'y') {
					cout << "保存中...";
					save();
					cout << "保存完毕！";
					ex = true;
				}
				break;
			}	
		}
	} while (!ex);	
	return 0;
}
