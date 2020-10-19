//tabucol, created on 2020/10/16
#include<iostream>
#include<vector>
#include<fstream>
#include<cstring>
#include<stdlib.h>
#include<string>
#include<time.h>
using namespace std;


vector<string> word;
int N;//结点数
int **graph;
int *neigh;
int **adj_color_table;
int **tabu_table;
int *sol;
int f;
int best_f;
int iter;
int K;
int colmove[1000][2];
int tabumove[1000][2];
int move_count;
int tabu_count;


//读文件，切分空格
void split(const string& delim, const string& src)
{
	word.clear();
	int start = 0,index;
	index = src.find(delim, start);
	while (index != string::npos)
	{
		word.push_back(src.substr(start, index - start));
		start = index + 1;
		index = src.find(delim, start);
	}
	word.push_back(src.substr(start, src.size() - start));
}

//分配内存，初始化图
void init_graph()
{
	graph = new int*[N];
	neigh = new int[N];
	for (int i = 0; i < N; i++)
	{
		graph[i] = new int[N];
		neigh[i] = 0;
	}
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < N; j++)
		{
			graph[i][j] = 0;
		}
	}
}

//建图
void create_graph(const string& filename)
{
	ifstream infile(filename, ios::in);
	string delim(" ");
	string textline;
	bool start = false;
	int tmp;
	int v1;
	int v2;
	while (infile.good())
	{
		getline(infile, textline);
		if (start&&textline.find("e",0)!=string::npos)
		{
			split(delim, textline);
			v1 = stoi(word[1])-1;
			v2 = stoi(word[2])-1;
			tmp = neigh[v1];
			graph[v1][tmp] = v2;
			neigh[v1]++;
			tmp = neigh[v2];
			graph[v2][tmp] = v1;
			neigh[v2]++;
		}
		else if (textline.find("p edge",0)!=string::npos)
		{
			split(delim, textline);
			N = stoi(word[2]);
			init_graph();
			start = true;
		}
	}
	infile.close();
}


//初始化table,分配内存
void initial_table()
{
	sol = new int[N];
	adj_color_table = new int*[N];
	tabu_table = new int*[N];
	for (int i = 0; i < N; i++)
	{
		adj_color_table[i] = new int[K];
		tabu_table[i] = new int[K];
	}
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < K; j++)
		{
			adj_color_table[i][j] = 0;
			tabu_table[i][j] = 0;
		}
	}
}

//随机分配结点颜色，初始化f和best_f
void initialization(int colornum)
{
	K = colornum;
	f = 0;
	for (int i = 0; i < N; i++)
	{
		sol[i] = rand() % K;
	}
	for (int i = 0; i < N; i++)
	{
		int edges = neigh[i];
		for (int j = 0; j < edges; j++)
		{
			if (sol[graph[i][j]] == sol[i])
			{
				f++;
			}
			adj_color_table[i][sol[graph[i][j]]]++;
		}
	}
	f /= 2;
	best_f = f;
	cout << "conflicts:" << f << endl;
}


int findmove()
{
	int delt = 100, tabu_delt = 100;
	int color;
	move_count = 0, tabu_count=0;
	for (int i = 0; i < N; i++)
	{
		color = sol[i];//获得结点颜色
		if (adj_color_table[i][color] > 0) 
		{
			for (int j = 0; j < K; j++)
			{
				if (j == color)
					continue;
				if (tabu_table[i][j] <= iter)  //j这个颜色属性不在禁忌期内
				{
					if (delt >= adj_color_table[i][j] - adj_color_table[i][color])
					{
						if (delt > adj_color_table[i][j] - adj_color_table[i][color])  //出现了更小的delt值
						{
							delt = adj_color_table[i][j] - adj_color_table[i][color];
							move_count = 0;
						}
						colmove[move_count][0] = i;
						colmove[move_count][1] = j;
						move_count++;
					}
				}
				else  //颜色属性在禁忌期内
				{
					if (adj_color_table[i][j] - adj_color_table[i][color] <= tabu_delt)
					{
						if (tabu_delt > adj_color_table[i][j] - adj_color_table[i][color])  //出现了更小的delt值
						{
							tabu_delt = adj_color_table[i][j] - adj_color_table[i][color];
							tabu_count = 0;
						}
						tabumove[tabu_count][0] = i;
						tabumove[tabu_count][1] = j;
						tabu_count++;
					}
				}
			}
		}
	}
	//cout << tabu_count << "tabu_delt:" << tabu_delt << endl;
	//cout << move_count << "move_delt:" << delt << endl;
	if (tabu_delt < delt&&tabu_delt + f < best_f) //选择禁忌属性
	{
		return 1;
	}
	else                                           //选择非禁忌属性
	{
		return 0;
	}
}
void makemove(int mode)   
{
	int sel,node,color,newcolor;
	if (mode == 1)
	{
		sel = rand() % tabu_count;  //随机选择一个禁忌属性的move
		node = tabumove[sel][0];
		color = sol[node];
		newcolor = tabumove[sel][1];
	}
	else
	{
		sel = rand() % move_count;
		node = colmove[sel][0];
		color = sol[node];
		newcolor = colmove[sel][1];
	}
	sol[node] = newcolor;
	f += adj_color_table[node][newcolor] - adj_color_table[node][color];
	if (f < best_f)
		best_f = f;
	tabu_table[node][color] = f + rand() % 10 + 1 + iter;
	//cout << "move:" << node << "from" << color << "to" << newcolor << endl;
	int j = 0;
	while (j<neigh[node])
	{
		int adjnode = graph[node][j];
		adj_color_table[adjnode][color]--;
		adj_color_table[adjnode][newcolor]++;
		j++;
	}
}
bool check()
{
	int color;
	for (int i = 0; i < N; i++)
	{
		color = sol[i];
		for (int j = 0; j < neigh[i]; j++)
		{
			int tmp = graph[i][j];
			if (sol[tmp] == color)
				return false;
		}

	}
	return true;
}
void tabusearch(const string& filename)
{
	cout << "input color number:";
	cin >> K;
	create_graph(filename);
	initial_table();
	initialization(K);
	int mode;
	iter = 0;
	srand(clock());
	cout << "Processing.." << endl;
	double start_time = clock(),end_time,iter_time;
	while (f > 0)
	{
		iter++;
		//system("pause");
		if (iter % 100000 == 0)
		{
			cout << "iter " << iter <<"  f:"<<f<< endl;
		}
		mode = findmove();
		makemove(mode);
	}
	end_time = clock();
	iter_time = (double(end_time - start_time)) / CLOCKS_PER_SEC;
	if (check())
		cout << "DONE by" << iter << "  time:" << iter_time << "s" << endl;
	else
		cout << "wrong answer" << endl;
}
int main()
{
	string file = "D:\\tabucol\\data\\DSJC500.5.col";
	while (true)
		tabusearch(file);
	system("pause");
	return 0;
}