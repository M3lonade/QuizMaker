
#include <iostream>
#include <string>
#include <cctype>
#include <chrono>
#include <fstream>
#include <filesystem>
#include <windows.h>

using namespace std;

void quizMain();
void consoleClear();
string stringToLower(string inputString);
bool isIdentificationValid(string identification);
bool isQuestionUnique(int questionId);
bool loadQuestionBank(vector<string>& questions, vector<string>& answers);
int getRandomQuestionId(int maxQuestions);

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// This code was coded & compiled in C++20 on Visual Studio 2019.
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

int main()
{
	// Give a random seed for rand() so it does not compute the same random values
	// every time that the application is run.
	srand(time(NULL));

	quizMain();
	cout << endl << endl << endl;

	while (true)
	{
		char input;
		cout << "Enter Q to exit or S to begin a new quiz for another student." << endl;
		cin >> input;

		input = tolower(input);
		if (input == 'q')
		{
			return 0;
		}
		else if (input == 's')
		{
			quizMain();
			cout << endl << endl << endl;
		}
		else
		{
			cout << "Invalid input!" << endl;
		}
	}
}

/**
 * @brief The main logic for the quiz. Off-loaded to a function to make it easier to allow starting new quizzes.
*/
void quizMain()
{
	bool bDetailsVerified = false;
	string studentFirstName, studentLastName;

	// In order to allow the users to both enter and verify their first and last name
	// we're going to verify in a loop whether or not the details were in fact verified.
	// The actual verification is a step that comes in later.
	while (!bDetailsVerified)
	{
		consoleClear();

		cout << "[Stage 1: General Details]" << endl;
		cout << "First Name: ";
		cin >> studentFirstName;
		cout << "Last Name: ";
		cin >> studentLastName;

		// This is the verification step, which asks the user whether or not the details
		// that they've entered are correct. We do this in a loop because it's a yes or no answer,
		// which means that we need to check if the reply was in fact a yes or a no.
		bool bReplyValid = false;
		while (!bReplyValid)
		{
			consoleClear();

			cout << "[Stage 1: General Details]" << endl;
			cout << "First Name: " << studentFirstName << endl;
			cout << "Last Name: " << studentLastName << endl;

			char verificationCheckReply;
			cout << "Are these details correct? (Y/N): ";
			cin >> verificationCheckReply;

			// Use "tolower" in order to allow both uppercase and lowercase replies (Y/y/N/n).
			// Set the booleans accordingly (which decide which loops should repeat, if any).
			verificationCheckReply = tolower(verificationCheckReply);
			if (verificationCheckReply == 'y')
			{
				bReplyValid = true;
				bDetailsVerified = true;
			}
			else if (verificationCheckReply == 'n')
			{
				bReplyValid = true;
				bDetailsVerified = false;
			}
		}
	}

	consoleClear();

	// Begin with 3 attempts (go from 3->0 and not 0->3 so we can display it to the user easily),
	// with each opportunity (at the end of the loop), decrement the attempts counter. This is fine
	// to do even if the user entered valid input as the loop should exit either if it is valid or
	// if the attempts counter is 0, we actually check and exit the program AFTER the loop rather
	// than doing it in here (which just complicates things).
	bool bIdentificationValid = false;
	string studentIdentification;
	int attempts = 3;
	while (!bIdentificationValid and attempts > 0)
	{
		cout << "[Stage 2: Identification]" << endl;
		cout << "Your identification number must begin with an 'A' followed by five numbers." << endl;
		cout << "You have " << attempts << " attempts left." << endl;
		cout << "Identification: ";
		cin >> studentIdentification;

		// The length of the string must be 6 (AXXXXX) and begin with an A.
		if (studentIdentification.length() == 6 and studentIdentification.at(0) == 'A')
		{
			bIdentificationValid = true;

			// Verify that all of the remaining parts of the string are indeed numbers.
			for (int i = 1; i < 5; ++i)
			{
				if (!isdigit(studentIdentification.at(i)))
				{
					bIdentificationValid = false;
					break;
				}
			}
		}

		--attempts;
	}

	// If the user failed to insert a correct identification in their three attempts, exit.
	if (!bIdentificationValid)
	{
		consoleClear();
		cout << "[Stage 2: Identification]" << endl;
		cout << "You have entered too many invalid identification serials!" << endl;
		return;
	}

	// Question count is defined 10 by default. The following code after that asks the user
	// Whether they want to double the question amount. If they make an invalid input the loop
	// Repeats itself until the user makes a valid input.
	int questionCount = 10;
	bool bQuestionCountValid = false;
	while (!bQuestionCountValid)
	{
		consoleClear();
		cout << "[Stage 3: Quiz Setup]" << endl;
		cout << "The current question count is 10." << endl;

		char questionCountReply;
		cout << "Would you like to double the question count to 20? (Y/N): ";
		cin >> questionCountReply;

		questionCountReply = tolower(questionCountReply);
		if (questionCountReply == 'y')
		{
			bQuestionCountValid = true;
			questionCount = 20;
		}
		else if (questionCountReply == 'n')
		{
			bQuestionCountValid = true;
		}
	}

	vector<string> Q;
	vector<string> A;
	if (!loadQuestionBank(Q, A))
	{
		consoleClear();
		cout << "Error: Failed to load question bank." << endl;
		return;
	}

	vector questionsUsed(Q.size(), false);

	// Leave it at 20 even if we only use 10, so we don't have to use dynamic arrays.
	int askedQuestions[20] = {};
	bool studentAnswers[20] = {};
	int questionsCorrect = 0;

	const chrono::time_point quizStartTime = chrono::high_resolution_clock::now();
	const chrono::time_point quizEndTime = quizStartTime + 10min;
	int questionsLeft = questionCount;
	int questionId = 0;
	while (questionsLeft > 0 and quizEndTime > chrono::high_resolution_clock::now())
	{
		int currentQuestion;
		do
		{
			currentQuestion = getRandomQuestionId(Q.size());
		} while (questionsUsed[currentQuestion]);

		questionsUsed[currentQuestion] = true;
		askedQuestions[questionId] = currentQuestion;

		bool bQuestionReplyValid = false;
		while (!bQuestionReplyValid)
		{
			consoleClear();

			cout << "[Stage 4: Quiz]" << endl;
			cout << "You have 10 minutes to complete the quiz. There are " << questionsLeft << " questions remaining." << endl;
			cout << "Valid replies are: True/False/T/F." << endl << endl;

			string questionReply;
			cout << "Question #" << questionId + 1 << ": " << Q[currentQuestion] << endl;
			cout << "Is this true or false?: ";
			cin >> questionReply;

			questionReply = stringToLower(questionReply);
			if (questionReply == "true" or questionReply == "t")
			{
				studentAnswers[questionId] = true;
				bQuestionReplyValid = true;
				if (A[currentQuestion] == "TRUE")
				{
					MessageBoxA(nullptr, "CORRECT MOTHERFUCKER", "IT WAS TRUE", MB_OK | MB_ICONINFORMATION);
					++questionsCorrect;
				}
			}
			else if (questionReply == "false" or questionReply == "f")
			{
				studentAnswers[questionId] = false;
				bQuestionReplyValid = true;
				if (A[currentQuestion] == "FALSE")
				{
					MessageBoxA(nullptr, "CORRECT MOTHERFUCKER", "IT WAS FALSE", MB_OK | MB_ICONINFORMATION);
					++questionsCorrect;
				}
			}
		}

		++questionId;
		--questionsLeft;
	}

	consoleClear();
	cout << "[Stage 5: Results]" << endl;
	if (quizEndTime <= chrono::high_resolution_clock::now())
	{
		cout << "The quiz ended because you ran out of time!" << endl;
	}
	else
	{
		cout << "You have completed the quiz." << endl;
	}

	float questionScore = 1.f;
	if (questionCount == 20)
	{
		questionScore = 0.5f;
	}

	const chrono::duration<float, nano> timeElapsed = chrono::high_resolution_clock::now() - quizStartTime;

	cout << "First Name: " << studentFirstName << endl;
	cout << "Last Name: " << studentLastName << endl;
	cout << "Score: " << questionScore * (float)questionsCorrect << "(" << ((float)questionsCorrect / (float)questionCount) * 100.f << "%)" << endl;
	cout << "Elapsed Time: " << chrono::duration_cast<chrono::minutes>(timeElapsed).count() << " minute(s)." << endl;
	cout << "Answer Sheet:" << endl;
	for (int i = 0; i < questionCount; ++i)
	{
		cout << "Question #" << i + 1 << ": " << Q[askedQuestions[i]] << " | Correct Answer: " << A[askedQuestions[i]] << " | Your answer: " << (studentAnswers[i] ? "TRUE" : "FALSE") << endl;
	}
}

/**
 * @brief Clears the console.
*/
void consoleClear()
{
#if _WIN32
	// If it's Windows, the command is cls.
	system("cls");
#else
	// If it's Linux, the command is clear.
	system("clear");
#endif
}

/**
 * @brief Returns a lowercase variant of the given string.
 * @param inputString The string that should be turned into lowercase.
 * @return A lowercase variant of the input string.
*/
string stringToLower(string inputString)
{
	string outputString = inputString;

	for (int i = 0; i < inputString.length(); ++i)
	{
		if (const char currentChar = outputString.at(i); currentChar <= 'Z' and currentChar >= 'A')
		{
			outputString.at(i) = currentChar + 32;
		}
	}

	return outputString;
}

bool isQuestionUnique(int questionId)
{
	return false;
}

bool loadQuestionBank(vector<string>& questions, vector<string>& answers)
{
	ifstream fileStream("TestBank.txt");
	if (!fileStream.is_open())
	{
		return false;
	}

	string currentLine;
	bool isQuestion = true;
	while (getline(fileStream, currentLine, '#'))
	{
		currentLine.erase(remove(currentLine.begin(), currentLine.end(), '\n'), currentLine.end());

		if (currentLine.length() != 0)
		{
			if (isQuestion)
			{
				questions.push_back(currentLine);
			}
			else
			{
				answers.push_back(currentLine);
			}

			isQuestion = !isQuestion;
		}
	}

	return true;
}

int getRandomQuestionId(int maxQuestions)
{
	return rand() % (maxQuestions - 1);
}
