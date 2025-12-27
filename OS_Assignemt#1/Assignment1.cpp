#include <iostream>
#include <map>
#include <string>
#include <fstream>
#include <vector>
#include <climits>
using namespace std;

enum processesState
{
	NEW,
	READY,
	RUNNING,
	TERMINATED
};

class process
{
public:
	process() {}
	process(int _PID, string _name, int _arriveTime, int _processTime) : 
	name{_name}, arriveTime{_arriveTime}, PID{_PID}, processTime{_processTime}, state{NEW}
	{
	}
	int PID;
	string name;
	int arriveTime;
	int processTime;
	processesState state;
};

class SchedulerResulte
{
public:
	SchedulerResulte() {}
	SchedulerResulte(process p) : _process(p.PID, p.name, p.arriveTime, p.processTime) {}
	process _process;
	int startTime;
	int endTime;

	int processOrder = 0;
};
void saveSchedulerResulte(string filePath, map<int, SchedulerResulte> SchedulerResultes)
{
	ofstream outputFile(filePath);
	if (!outputFile.is_open())
	{
		cout << "Could not open file " << filePath << "\n";
		return;
	}
	// first we want to store the order of execution for the processes
	for (int i = 0; i < SchedulerResultes.size(); i++)
	{
		auto it = SchedulerResultes.begin();
		while (it != SchedulerResultes.end())
		{
			if (it->second.processOrder == i)
				outputFile << it->second._process.name;
			it++;
		}
	}
	outputFile << endl<< endl;

	// second we will save all the date in this format name: (response,turnaround,delay)
	auto it = SchedulerResultes.begin();
	while (it != SchedulerResultes.end())
	{
		int response = it->second.startTime - it->second._process.arriveTime;
		int turnaround = it->second.endTime - it->second._process.arriveTime;
		int delay = turnaround - it->second._process.processTime;
		outputFile << it->second._process.name << ":(";
		outputFile << "response=" << response << ',';
		outputFile << "turnaround=" << turnaround << ',';
		outputFile << "delay=" << delay << ')';
		outputFile << endl;
		it++;
	}
	outputFile.close();
}
void loadProcessesInput(string filePath, vector<process> &processes)
{

	ifstream inputFile(filePath);

	if (!inputFile.is_open())
	{
		cout << "Error opening the file\n";
		return;
	}
	int currPID = 2;
	int n = 0;
	// load the number of processes from input file(its the first word in the text file)
	inputFile >> n;
	string procName;
	int arrTime = 0;
	int procTime = 0;
	// load the processes data (name,arrive time,processing time)
	for (int i = 0; i < n; i++)
	{
		inputFile >> procName;
		inputFile >> arrTime;
		inputFile >> procTime;
		process newProcess(currPID++, procName, arrTime, procTime);
		processes.push_back(newProcess);
	}
	// close the input file
	inputFile.close();
}

void SJF_Scheduler(vector<process> processes)
{
	// to save the resulte of the Scheduler
	map<int, SchedulerResulte> SchedulerResultes;
	int processCounterOrder = 0;
	int currentTime = 0;

	process *currentProcess = nullptr;
	bool exit = false;
	int mn = INT_MAX;
	while (!exit)
	{
		// check if all processes are terminated
		bool notYet = false;
		for (int j = 0; j < processes.size(); j++)
		{
			if (processes[j].state != TERMINATED)
			{
				notYet = true;
			}
		}
		if (notYet == false)
		{
			exit = true;
			continue;
		}
		// check if any process arrived and put it in the ready state
		for (int j = 0; j < processes.size(); j++)
		{
			if (currentTime == processes[j].arriveTime)
			{
				processes[j].state = READY;
				SchedulerResulte procRes(processes[j]);
				SchedulerResultes.insert({processes[j].PID, procRes});
			}
		}
		// check if a process is running if not pick the shortest one
		if (currentProcess == nullptr)
		{

			for (int j = 0; j < processes.size(); j++)
			{
				if (processes[j].processTime < mn && processes[j].state == READY)
				{
					currentProcess = &processes[j];
					mn = processes[j].processTime;
				}
			}
			if (currentProcess != nullptr)
			{
				currentProcess->state = RUNNING;
				SchedulerResultes[currentProcess->PID].processOrder = processCounterOrder++;
				SchedulerResultes[currentProcess->PID].startTime = currentTime;
			}
		}
		// execute the runing process
		if (currentProcess != nullptr)
		{
			currentProcess->processTime--;
			if (currentProcess->processTime == 0)
			{
				currentProcess->state = TERMINATED;
				SchedulerResultes[currentProcess->PID].endTime = currentTime + 1;
				currentProcess = nullptr;
				mn = INT_MAX;
			}
		}
		currentTime++;
	}
	saveSchedulerResulte("out.txt", SchedulerResultes);
}

int main()
{
	// input
	vector<process> processes;
	loadProcessesInput("in.txt", processes);
	//solution
	SJF_Scheduler(processes);
	return 0;
}
