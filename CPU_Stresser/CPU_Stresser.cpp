#include <windows.h>
#include <winbase.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <math.h>

volatile bool RunStressProcess = false;

static DWORD WINAPI  StressProcess(LPVOID lpParam)
{
	do
	{
		long double f[] = { sin(0.000000001) ,sin(0.000000001) };
	} while (RunStressProcess == true);
    return 0;
}

int main()
{
	std::cout << ("CPUストレステストプログラム\n");
	std::cout << ("　　　　 2024/04/13 S.Yukisita\n\n");

	//諸々変数定義
	char choice[100];
	char choiceN[] = "n";
	char choiceY[] = "y";

	//論理プロセッサー数を数える
	SYSTEM_INFO SysInfo;
	GetNativeSystemInfo(&SysInfo);
	printf ("論理プロセッサ数: %d\n", SysInfo.dwNumberOfProcessors);

START:
	//負荷を加えるスレッド数の設定
	int TargetCPU = 0;
	while (1)
	{
		std::cout << ("ストレステストを実行するスレッド数を入力してください：");
		scanf_s("%d", &TargetCPU);
		if (TargetCPU <= 0 || SysInfo.dwNumberOfProcessors < TargetCPU)
		{
			std::cout << ("無効な数字が入力されました。\n再度");
		}
		else
		{
			break;
		}
	}

	//負荷を加える論理CPUが1スレッドの場合、スレッド番号を選択
	int CPUNumber = 0;
	if (TargetCPU == 1)
	{
		while (1)
		{
			std::cout << ("ストレステストを実行するスレッドの番号を入力してください：");
			scanf_s("%d", &CPUNumber);
			if (CPUNumber <= 0 || SysInfo.dwNumberOfProcessors < CPUNumber)
			{
				std::cout << ("無効な数字が入力されました。\n再度");
			}
			else
			{
				CPUNumber = CPUNumber - 1;
				break;
			}
		}
	}

	//負荷を加える時間を設定
	int Duration;
	std::cout << ("ストレステストを継続する時間を設定しますか？（Y/N）:");
	do
	{
		scanf_s("%s", choice, 100);
		if (strcmp(choice, choiceY) != 0 && strcmp(choice, choiceN) != 0)
		{
			std::cout << "yまたはnを入力してください。：";
		}
	} while (strcmp(choice, choiceY) != 0 && strcmp(choice, choiceN) != 0);
	if (strcmp(choice, choiceY) == 0)
	{
		std::cout << ("ストレステストを継続する時間を入力してください（秒）：");
		scanf_s("%d", &Duration);
		if (Duration < 0)
		{
			std::cout << ("無効な数字が入力されました");
			return(0);
		}
	}
	else
	{
		Duration = 0;
	}

	//負荷の開始
	RunStressProcess = true;
	if (TargetCPU == 1) //1コア用
	{
		DWORD dw_TheadID;
		HANDLE h_Thead;
		h_Thead = CreateThread(NULL, 0, StressProcess, 0, 0, &dw_TheadID);
		SetThreadAffinityMask(h_Thead, static_cast<DWORD_PTR>(1) << CPUNumber);
		//待機
		if (Duration == 0)
		{
			int button1S = MessageBox(NULL,
				TEXT("ストレステストを終了します"),
				TEXT("CPU_StressTest"),
				MB_OK | MB_ICONSTOP);
		}
		else
		{
			printf(" Time Remaining --> %ds", Duration);
			for (int TimeCount = 1; TimeCount <= Duration; TimeCount += 1)
			{
				Sleep(1000);
				printf("\r Time Remaining --> %ds   ", Duration - TimeCount);
			}
			std::cout << ("\n");
		}

		//負荷の終了
		RunStressProcess = false;
		WaitForSingleObject(h_Thead, INFINITE);
		CloseHandle(h_Thead);
	}
	else//マルチコア用
	{
		HANDLE* h_TheadArray;
		h_TheadArray = new HANDLE[TargetCPU];
		DWORD* dw_TheadIDArray;
		dw_TheadIDArray = new DWORD[TargetCPU];
		int i = 0;
		for (i = 0; i < TargetCPU; i += 1)
		{
			h_TheadArray[i] = CreateThread(NULL, 0, StressProcess, 0, 0, &dw_TheadIDArray[i]);
			SetThreadAffinityMask(h_TheadArray[i], static_cast<DWORD_PTR>(1) << i);
		}
		//待機
		if (Duration == 0)
		{
			int button1M = MessageBox(NULL,
				TEXT("ストレステストを終了します"),
				TEXT("CPU_StressTest"),
				MB_OK | MB_ICONSTOP);
		}
		else
		{
			printf(" Time Remaining --> %ds", Duration);
			for (int TimeCount = 1; TimeCount <= Duration; TimeCount += 1)
			{
				Sleep(1000);
				printf("\r Time Remaining --> %ds   ", Duration - TimeCount);
			}
			std::cout << ("\n");
		}
		//負荷の終了
		RunStressProcess = false;
		for (i = 0; i < TargetCPU; i += 1)
		{
			WaitForSingleObject(h_TheadArray[i], INFINITE);
			CloseHandle(h_TheadArray[i]);
		}
		delete[] h_TheadArray;
		delete[] dw_TheadIDArray;
	}
	std::cout << "**************************************************************************************" << std::endl << std::endl;
	goto START;
}