// DistributedSystemSimulator.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Node.h"
#include "MasterSingleton.h"
#include "Log.h"

//FOR MEMORY LEAKS
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>


#include "MemPage.h"

void test_single_node();
void test_node_time();
void test_file_config_process();
void test_file_config_node();
void run_test_suite();
void run_2_nodes();
void test_file_config_node_with_singleton();
void run_1_node();
void test_mem_leak();
void basic_testcase_1_cpu_2_nodes_2_procs();
void basic_testcase_2_mem_2_nodes_2_procs();

int _tmain(int argc, _TCHAR* argv[])
{
	CMasterSingleton::GetInstance()->SystemBringUp();
	CMasterSingleton::GetInstance()->RunToTime();
	return 0;
}

void test_mem_leak()
{
	for (int i = 0; i < 50; ++i)
	{
		CNode node1("Config/Node1Real.txt");
	}

}

void basic_testcase_1_cpu_2_nodes_2_procs()
{
	// In this testcase, we can see a machine with high CPU usage migrating a process to another machine
	//CNode *node1 = new CNode("Config/Node1Real.txt");
	//CNode *node2 = new CNode("Config/Node2Real.txt");
	//
	//node1->AddProcess(new CProcess("Config/Process6.txt"));
	//node1->AddProcess(new CProcess("Config/Process6.txt"));
	//
	//CMasterSingleton::GetInstance()->AddNode(node1);
	//CMasterSingleton::GetInstance()->AddNode(node2);
	CMasterSingleton::GetInstance()->SystemBringUp();
	CMasterSingleton::GetInstance()->RunToTime();
}

void basic_testcase_2_mem_2_nodes_2_procs()
{
	// In this testcase, we can see a machine with high CPU usage migrating a process to another machine
	CNode *node1 = new CNode("Config/Node1Real.txt");
	CNode *node2 = new CNode("Config/Node2Real.txt");

	node1->AddProcess(new CProcess("Config/Process7.txt"));
	node1->AddProcess(new CProcess("Config/Process8.txt"));

	CMasterSingleton::GetInstance()->AddNode(node1);
	CMasterSingleton::GetInstance()->AddNode(node2);
	CMasterSingleton::GetInstance()->SetStatusReportCycle(40);
	CMasterSingleton::GetInstance()->SetEnableMigration(true);
	CMasterSingleton::GetInstance()->SetMemoryMigrationTreshold(0.75);
	CMasterSingleton::GetInstance()->RunToTime();
}

void run_2_nodes()
{
	CNode *node1 = new CNode("Config/Node1Real.txt");
	CNode *node2 = new CNode("Config/Node2Real.txt");

	node1->AddProcess(new CProcess("Config/Process1.txt"));
	node1->AddProcess(new CProcess("Config/Process2.txt"));
	node2->AddProcess(new CProcess("Config/Process3.txt"));

	CMasterSingleton::GetInstance()->AddNode(node1);
	CMasterSingleton::GetInstance()->AddNode(node2);
	CMasterSingleton::GetInstance()->SetStatusReportCycle(200);
	CMasterSingleton::GetInstance()->SetEnableMigration(false);
	CMasterSingleton::GetInstance()->RunToTime();
}

void run_1_node()
{
	CNode node("Config/Node3Real.txt");

	node.AddProcess(new CProcess("Config/Process3.txt"));
	node.AddProcess(new CProcess("Config/Process4.txt"));
	bool o_deadline;
	node.Init();
	node.RunToTime(500, o_deadline);
	node.PushRun();
}

void run_test_suite()
{
	test_single_node();
	test_node_time();
	test_file_config_process();
	test_file_config_node();
	test_file_config_node_with_singleton();
}


void test_single_node()
{
	uint16_t process_id = 0;

	CCodeBase codeBase;
	codeBase.AddCommand(false, 100);
	codeBase.AddCommand(true, 150, 0x500);
	codeBase.AddCommand(false, 380);
	codeBase.AddCommand(true, 150, 0x800);

	CMasterSingleton::GetInstance()->MainLog("Testing command object");
	assert(codeBase.GetCommandCount() == 4);
	assert(codeBase.GetMaxAddr() == 2197); //0x800 + 150 - 1
	CMasterSingleton::GetInstance()->MainLog("Pass");

	CMasterSingleton::GetInstance()->MainLog("Testing process object");
	CProcess process(codeBase, 0x1000, 0x1000);
	assert(process.GetId() == 0);
	assert(process.GetDeadline() == 0x1000);
	assert(process.GetMemoryRequired() == 0x1000);
	CProcess process2(codeBase, 0x1000, 0x1000);
	assert(process2.GetId() == 1);
	assert(process2.GetDeadline() == 0x1000);
	assert(process2.GetMemoryRequired() == 0x1000);
	// Test that the constructor throw is working as expected
	codeBase.AddCommand(true, 500, 0x1000);
	CMasterSingleton::GetInstance()->MainLog("Checking that exception is properly thrown");
	try {
		CProcess process3(codeBase, 0x1000, 0x1000);
		// if the exception is not thrown, this assert should kill us.
		assert(1 == 0);
	}
	catch (CProcessException *e) {
		assert(e->GetExceptionInfo() == CProcessException::kAddressOutsideRange);
	}
	CMasterSingleton::GetInstance()->MainLog("Pass");

	CMasterSingleton::GetInstance()->MainLog("Testing node object with page allocation");
	double freq = 2.9;
	CNode node(8, 2, freq);
	CCodeBase codeBaseDummy;
	codeBase.AddCommand(false, 10);
	CProcess* pProcess;
	if (!node.AddProcess(new CProcess(codeBaseDummy, 4, 30)))
		assert(false);
	if (!node.AddProcess(new CProcess(codeBaseDummy, 2, 30)))
		assert(false);
	if (!node.AddProcess(new CProcess(codeBaseDummy, 2, 30)))
		assert(false);
	pProcess = new CProcess(codeBaseDummy, 2, 30);
	if (node.AddProcess(pProcess))
		assert(false);
	//remove process 3, 5, and then add another process of size 6 to see that it gets all the pages.
	node.RemoveProcess(3);
	node.RemoveProcess(5);
	if (!node.AddProcess(new CProcess(codeBaseDummy, 6, 30)))
		assert(false);
	CMasterSingleton::GetInstance()->MainLog("Pass");
}

void test_node_time()
{
	bool deadline = false;
	CNode node = CNode(100, 10, 2.9);
	node.RunToTime(30, deadline);
	node.PushRun();
	CMasterSingleton::GetInstance()->MainLog(std::to_string(node.GetTime()));
	node.RunToTime(55, deadline);
	node.PushRun();
	CMasterSingleton::GetInstance()->MainLog(std::to_string(node.GetTime()));

	try {
		node.RunToTime(50, deadline);
		// if the exception is not thrown, this assert should kill us.
		assert(1 == 0);
	}
	catch (CNodeException *e) {
		assert(e->GetExceptionInfo() == CNodeException::kCannotGoBackInTime);
	}

	node.RunToTime(75, deadline);
	node.PushRun();
	try {
		node.PushRun();
		// if the exception is not thrown, this assert should kill us.
		assert(1 == 0);
	}
	catch (CNodeException *e) {
		assert(e->GetExceptionInfo() == CNodeException::kCannotPushNothing);
	}
	CMasterSingleton::GetInstance()->MainLog(std::to_string(node.GetTime()));
}

void test_file_config_process()
{
	std::ifstream input_file("Config/Node.txt");

	CProcess process("Config/Process1.txt");
	
	try
	{
		CProcess process("Config/process_deadline_exception.txt");
		assert(1 == 0);
	}
	catch (CProcessException *e)
	{
		assert(e->GetExceptionInfo() == CProcessException::kDeadlineNotSet);
	}

	try
	{
		CProcess process1("Config/process_expected_config_base_exception.txt");
		assert(1 == 0);
	}
	catch (CProcessException *e)
	{
		assert(e->GetExceptionInfo() == CProcessException::kExpectedCodeBaseConfig);
	}

	try
	{
		CProcess process1("Config/process_memory_too_small_exception.txt");
		assert(1 == 0);
	}
	catch (CProcessException *e)
	{
		assert(e->GetExceptionInfo() == CProcessException::kAddressOutsideRange);
	}

	try
	{
		CProcess process1("Config/process_required_memory_exception.txt");
		assert(1 == 0);
	}
	catch (CProcessException *e)
	{
		assert(e->GetExceptionInfo() == CProcessException::kMemoryRequiredNotSet);
	}

	try
	{
		CProcess process1("Config/process_starttime_exception.txt");
		assert(1 == 0);
	}
	catch (CProcessException *e)
	{
		assert(e->GetExceptionInfo() == CProcessException::kStartTimeNotSet);
	}
}

void test_file_config_node()
{
	std::ifstream input_file("Config/Node.txt");

	CNode node("Config/Node.txt");
	if (!node.AddProcess(new CProcess("Config/Process1.txt")))
		assert(0 == 1);
	if (!node.AddProcess(new CProcess("Config/Process1.txt")))
		assert(0 == 1);
	if (!node.AddProcess(new CProcess("Config/Process1.txt")))
		assert(0 == 1);
	if (!node.AddProcess(new CProcess("Config/Process1.txt")))
		node.AddProcess(new CProcess("Config/Process2.txt"));
}

void test_file_config_node_with_singleton()
{
	std::ifstream input_file("Config/Node.txt");

	CNode node("Config/Node.txt");
	if (!node.AddProcess(new CProcess("Config/Process1.txt")))
		assert(0 == 1);
	if (!node.AddProcess(new CProcess("Config/Process1.txt")))
		assert(0 == 1);
	if (!node.AddProcess(new CProcess("Config/Process1.txt")))
		assert(0 == 1);
	if (!node.AddProcess(new CProcess("Config/Process1.txt")))
		node.AddProcess(new CProcess("Config/Process2.txt"));

	node.WriteLog("TEST");
	CMasterSingleton::GetInstance()->MainLog("TEST 2");
	
}