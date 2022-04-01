
#include <iostream>
#include <string>
#include <cctype>
#include <chrono>
#include <fstream>
#include <filesystem>

using namespace std;

// Functions are documented at their implementation.
void quizMain();
void consoleClear();
string stringToLower(string inputString);
bool isIdentificationValid(string identification);
bool isQuestionUnique(vector<bool>& questionsUsed, int questionId);
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

	// Launch the quiz first - we don't want the option list to show up until after
	// the first quiz has been completed (as per the design document).
	quizMain();
	cout << endl << endl << endl;

	// This is an infinite loop since we're waiting for either correct input (which will then put us in quizMain -or- exit
	// If there is no correct input, we just want to print out the mistake and repeat the loop.
	while (true)
	{
		char input;
		cout << "Enter Q to exit or S to begin a new quiz for another student." << endl;
		cin >> input;

		// Allow Q/q/S/s just in case.
		input = tolower(input);
		if (input == 'q')
		{
			return 0;
		}
		else if (input == 's')
		{
			// Repeat the quiz. Put some spaces after so it aligns nicely the same
			// way it did after the first quiz. :)
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

		bIdentificationValid = isIdentificationValid(studentIdentification);
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

	// The vector Q and vector A are passed into loadQuestionBank as references and the questions/answers
	// are consequently loaded into them. These dynamic arrays are not expected to change after the call
	// as consequent data is saved on different arrays.
	// This is loaded here (as opposed to in main, for example) in order to allow runtime changes of the
	// test bank, primarily for testing purposes. This means that after the quiz is completed, if the
	// TestBank.txt file was modified during this time, once 's' is entered - the new questions and
	// answers will immediately work with no need to rebuild or hotload the changes into the program.
	vector<string> Q;
	vector<string> A;
	if (!loadQuestionBank(Q, A))
	{
		// If we fail to load the question bank, we can't run the quiz, so might as well exit out of
		// the quiz logic.
		consoleClear();
		cout << "Error: Failed to load question bank." << endl;
		return;
	}

	// This is the dynamic array that is used to map a question ID to a boolean value indicating on
	// whether or not the question was already used in the past. It cannot be a simple C array or even
	// a standard library array because its size is initialized at run-time and depends on the amount
	// of questions present at the test bank at any given time.
	vector questionsUsed(Q.size(), false);

	// Leave it at 20 even if we only use 10, so we don't have to use dynamic arrays here as well.
	// Generally speaking, this is possible since we know the highest possible amount of questions
	// asked during the quiz will never exceed 20, unlike the test bank which could go well into
	// the thousands of questions.
	int askedQuestions[20] = {};
	bool studentAnswers[20] = {};

	// The amount of correct questions, used to calculate the score without re-iterating over all
	// of the questions and answers given by the student.
	int questionsCorrect = 0;

	// Since the quiz now begins, we can already calculate the start time (used to present the
	// elapsed time in the conclusion of the quiz) as well as the end time. We calculate the end time
	// of the quiz prior rather than constantly calculating it based on quizStartTime in the condition
	// of the while loop since we want to avoid re-calculating it with every iteration of the loop.
	const chrono::time_point quizStartTime = chrono::high_resolution_clock::now();
	const chrono::time_point quizEndTime = quizStartTime + 10min;
	int questionsLeft = questionCount;

	// We can calculate the current question ID based on questionsLeft and so forth (or the other
	// way around), but it is simply easier to keep track of another variable as opposed to performing
	// these calculations whenever we want to index one of the two.
	int questionId = 0;

	// Use a while loop in here since we're also reliant on the timer. Unfortunately, we have to check
	// the timer at the moment an answer is entered and not at all times because our code execution
	// is blocked while we are waiting for input. Threads could be used to solve this problem by
	// allowing us to both wait for input and query the current time, however, using them in C++ is
	// rather complex and doing it safely is even harder.
	while (questionsLeft > 0 and quizEndTime > chrono::high_resolution_clock::now())
	{
		// Get a random question ID in a loop until isQuestionUnique returns true.
		int currentQuestion;
		do
		{
			currentQuestion = getRandomQuestionId(Q.size());
		} while (!isQuestionUnique(questionsUsed, currentQuestion));

		// Mark the question as used and add it to the list of asked questions.
		// Adding it to the list of asked questions at the current question ID is needed in order
		// to display the questions at the conclusion of the quiz in the same order that they were
		// displayed originally and using the same indexes (1-20) rather than their actual ID as
		// registered on the Q array (from the test bank).
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

			// Lower the string to allow every variation of true/t/false/f lowercase/uppercase.
			questionReply = stringToLower(questionReply);
			if (questionReply == "true" or questionReply == "t")
			{
				// Mark that the student answered "true" to this question.
				studentAnswers[questionId] = true;
				bQuestionReplyValid = true;

				// The result is still "TRUE" on the test bank.
				if (A[currentQuestion] == "TRUE")
				{
					++questionsCorrect;
				}
			}
			else if (questionReply == "false" or questionReply == "f")
			{
				// Mark that the student answered "false" to this question.
				studentAnswers[questionId] = false;
				bQuestionReplyValid = true;

				// As above, the result is still "FALSE" on the test bank.
				if (A[currentQuestion] == "FALSE")
				{
					++questionsCorrect;
				}
			}
		}

		// Increase the question ID and decrease questions left.
		// As you can see, they are exact opposites of each other (and can thus be
		// calculated based on each-other), but for simplicity it was kept using
		// two variables.
		++questionId;
		--questionsLeft;
	}

	consoleClear();
	cout << "[Stage 5: Results]" << endl;

	// If the reason for the quiz ending was the time running out, let them know
	// about it. If the quiz was ended on the spot - milliseconds away from the
	// time expiring, this could accidentally print anyways, however, the chances
	// of that happening are very minor and thus for simplicity this check is
	// preferred over, say, a variable notifying us of that being the reason.
	if (quizEndTime <= chrono::high_resolution_clock::now())
	{
		cout << "The quiz ended because you ran out of time!" << endl;
	}
	else
	{
		cout << "You have completed the quiz." << endl;
	}

	// As per the design document, if the question count was doubled to 20, the
	// score of each question is halved (to 0.5).
	float questionScore = 1.f;
	if (questionCount == 20)
	{
		questionScore = 0.5f;
	}

	// Calculate the elapsed time. Notable, this can say 10 minutes if the quiz was ended with milliseconds
	// to spare, however, due to the chance of this being very minor, this is a simpler way to perform this.
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

	// Iterate over every character in the string.
	for (int i = 0; i < inputString.length(); ++i)
	{
		// This is simple string manipulation based on the ASCII table - considering the fact that
		// characters in C and C++ are simply numbers, we can perform arithmetic actions on them,
		// such as comparison, addition and so forth. Notably, this can be used to check if a given
		// character is uppercase, by checking if it is within the range ['A', 'Z'], which translates
		// to [65, 90].
		if (const char currentChar = outputString.at(i); currentChar <= 'Z' and currentChar >= 'A')
		{
			// The numerical difference between uppercase and lowercase numbers on the ASCII table is
			// equal to 32, so we can simply add 32 to the current character.
			outputString.at(i) += 32;
		}
	}

	return outputString;
}

/**
 * @brief Checks and returns whether a given student identification is valid.
 * @param identification The identification code to check.
 * @return True if the identification is valid, false otherwise.
*/
bool isIdentificationValid(string identification)
{
	// The length of the string must be 6 (AXXXXX) and it must begin with an 'A'.
	if (identification.length() != 6 or identification.at(0) != 'A')
	{
		return false;
	}

	// Verify that all of the remaining parts of the string are indeed numbers.
	for (int i = 1; i < 5; ++i)
	{
		if (!isdigit(identification.at(i)))
		{
			return false;
		}
	}

	return true;
}

/**
 * @brief Checks whether or not a given question is unique (hasn't been used for the current quiz).
 * @param questionsUsed The vector of questions which have already been used.
 * @param questionId The question identifier.
 * @return Whether or not the given question is unique.
*/
bool isQuestionUnique(vector<bool>& questionsUsed, int questionId)
{
	// All questions in the questionsUsed array have been defaulted to zero so we can
	// avoid bothering with bounds checking on the vector.
	return !questionsUsed[questionId];
}

/**
 * @brief Loads the question bank from the file TestBank.txt into the given vectors.
 * @param questions The vector to fill with the questions.
 * @param answers The vector to fill with the answers.
 * @return True if the question bank has been loaded, false otherwise.
*/
bool loadQuestionBank(vector<string>& questions, vector<string>& answers)
{
	// Create an input-file-stream to the file TestBank.txt, which should be
	// present with every copy of the executable. If we fail to open it,
	// return false, as per the function documentation.
	ifstream fileStream("TestBank.txt");
	if (!fileStream.is_open())
	{
		return false;
	}

	string currentLine;
	bool isQuestion = true;

	// Iterate over the entire file using the getline function, which actually
	// allows you to iterate using characters other than the newline delimiter.
	// Considering the structure of the given TestBank.txt file, using hashtag
	// as the delimiter is easier for us, as it means we get question-answer
	// in a loop.
	while (getline(fileStream, currentLine, '#'))
	{
		// Because we are using getline with the '#' (hashtag) delimiter, we have to get rid of the newline characters.
		// Technically, we could avoid this stage, but it is necessary in order to make the quiz look good.
		// Here we are using the common C++ erase-remove idiom in order to modify the string and remove every instance
		// of the newline character from within it, using standard library iterators.
		currentLine.erase(remove(currentLine.begin(), currentLine.end(), '\n'), currentLine.end());

		// Sometimes there may be empty lines in the file, such as the first or last line.
		// If we've come across such a line - move on, it is completely useless to us.
		if (currentLine.length() != 0)
		{
			// Use the isQuestion boolean to decide whether to add the value to the
			// questions or answers vector.
			if (isQuestion)
			{
				questions.push_back(currentLine);
			}
			else
			{
				answers.push_back(currentLine);
			}

			// The first value we get will always be a question, followed by an answer in the next
			// iteration. This will repeat until we are out of lines in the file.
			isQuestion = !isQuestion;
		}
	}

	// Return true if we've managed to open the file successfully.
	// Should note that the function may also return true if the TestBank.txt file
	// does not contain any questions or answers or is invalid in format.
	return true;
}

/**
 * @brief Returns a random question identifier to load from the test bank.
 * @param maxQuestions The maximum amount of valid questions.
 * @return A random number in the range [0, maxQuestions-1].
*/
int getRandomQuestionId(int maxQuestions)
{
	// Using rand() % n allows us to get a random value in the range [0, n].
	// In this case, we need it to be in the range of [0, maxQuestions-1].
	// The minus one is necessary since we start at 0 but maxQuestions
	// could be 10 or 20.
	return rand() % (maxQuestions - 1);
}
