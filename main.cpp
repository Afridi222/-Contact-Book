#include <iostream>
#include <fstream>
#include <sstream>
#include <queue>
#include <stack>
#include <iomanip>
#include <string>
#include <limits.h>
using namespace std;

struct Node
{
    string name;
    string phone_number;
    Node *next;
    Node *prev;
};

class ContactBook
{
    Node *head;
    queue<Node *> contactQueue;
    stack<Node *> addedContacts;
    stack<Node *> deletedContacts;
    const string passwordFile = "password.txt";
    bool authenticated = false;

    int simpleHash(const int password[], int length)
    {
        const int prime = 31;
        int hashValue = 17;

        for (int i = 0; i < length; ++i)
        {
            hashValue = (hashValue * prime + password[i]) % INT_MAX;
        }

        return hashValue;
    }

    bool CheckPassword(const int enteredPassword[], int length)
    {
        ifstream passfile(passwordFile);
        if (passfile.is_open() && passfile.peek() != EOF)
        {
            int storedHashedPassword;
            passfile >> storedHashedPassword;
            passfile.close();

            int enteredHashedPassword = simpleHash(enteredPassword, length);

            return enteredHashedPassword == storedHashedPassword;
        }
        else
        {
            return false;
        }
    }

    void SavePassword(const int password[], int length)
    {
        ofstream passfile(passwordFile);
        if (passfile.is_open())
        {
            int hashedPassword = simpleHash(password, length);
            passfile << hashedPassword;
            passfile.close();
        }
    }

    Node *SearchByName(Node *root, const string &name)
    {
        if (root == nullptr || root->name == name)
        {
            return root;
        }

        if (name < root->name)
        {
            return SearchByName(root->prev, name);
        }
        else
        {
            return SearchByName(root->next, name);
        }
    }

    Node *SearchByNumber(Node *root, const string &number)
    {
        if (root == nullptr || root->phone_number == number)
        {
            return root;
        }

        Node *leftResult = SearchByNumber(root->prev, number);
        if (leftResult != nullptr)
        {
            return leftResult;
        }

        return SearchByNumber(root->next, number);
    }

public:
    ContactBook() : head(nullptr), authenticated(false) {}

    ~ContactBook()
    {
        while (head != nullptr)
        {
            Node *temp = head;
            head = head->next;
            delete temp;
        }
    }

    void AddToQueue()
    {
        Node *newNode = new Node;
        cin.ignore();
        cout << "  Name of Contact: ";
        getline(cin, newNode->name);
        cout << "  Phone Number: ";
        getline(cin, newNode->phone_number);
        newNode->next = nullptr;
        newNode->prev = nullptr;
        contactQueue.push(newNode);
        cout << "  Contact added to queue." << endl;
    }

    void TransferFromQueue()
    {
        while (!contactQueue.empty())
        {
            Node *newNode = contactQueue.front();
            contactQueue.pop();

            if (head == nullptr)
            {
                head = newNode;
            }
            else
            {
                Node *temp = head;
                Node *prev = nullptr;

                while (temp != nullptr && temp->name < newNode->name)
                {
                    prev = temp;
                    temp = temp->next;
                }

                if (prev == nullptr)
                {
                    newNode->next = head;
                    head->prev = newNode;
                    head = newNode;
                }
                else
                {
                    newNode->next = prev->next;
                    newNode->prev = prev;
                    if (prev->next != nullptr)
                    {
                        prev->next->prev = newNode;
                    }
                    prev->next = newNode;
                }
            }
            addedContacts.push(newNode);
        }
        SaveToFile();
        cout << "  Contacts transferred from queue to contact book." << endl;
    }

    void Display()
    {
        Node *temp = head;
        int count = 0;

        if (temp == nullptr)
        {
            cout << "  No Contacts... Please Add Some Contacts" << endl;
            cout << "  Thanks" << endl;
        }
        else
        {
            cout << "  Name:              Number: " << endl;
            cout << "--------------------------------" << endl;
            while (temp != nullptr)
            {
                count++;
                cout << "  " << setw(20) << left << temp->name;
                cout << setw(15) << right << temp->phone_number << endl;
                temp = temp->next;
            }
            cout << "  Total contacts: " << count << endl;
        }
    }

    void DeleteContact()
    {
        if (head == nullptr)
        {
            cout << "  No contacts to delete." << endl;
            return;
        }

        string nameToDelete;
        cin.ignore();
        cout << "  Enter the name of the contact to delete: ";
        getline(cin, nameToDelete);

        Node *temp = head;
        while (temp != nullptr && temp->name != nameToDelete)
        {
            temp = temp->next;
        }

        if (temp == nullptr)
        {
            cout << "  Contact not found." << endl;
            return;
        }

        if (temp->prev != nullptr)
        {
            temp->prev->next = temp->next;
        }
        else
        {
            head = temp->next;
        }

        if (temp->next != nullptr)
        {
            temp->next->prev = temp->prev;
        }

        deletedContacts.push(temp);
        cout << "  Contact deleted." << endl;
        SaveToFile();
    }

    void UndoLastAddition()
    {
        if (!addedContacts.empty())
        {
            Node *lastAdded = addedContacts.top();
            addedContacts.pop();

            if (head == lastAdded)
            {
                head = head->next;
                if (head)
                    head->prev = nullptr;
            }
            else
            {
                Node *temp = head;
                while (temp != nullptr && temp->next != lastAdded)
                {
                    temp = temp->next;
                }
                if (temp != nullptr)
                {
                    temp->next = lastAdded->next;
                    if (lastAdded->next != nullptr)
                    {
                        lastAdded->next->prev = temp;
                    }
                }
            }
            deletedContacts.push(lastAdded);
            cout << "  Last added contact removed." << endl;
            SaveToFile();
        }
        else
        {
            cout << "  No contact to undo." << endl;
        }
    }

    void UndoLastDeletion()
    {
        if (!deletedContacts.empty())
        {
            Node *lastDeleted = deletedContacts.top();
            deletedContacts.pop();

            if (head == nullptr)
            {
                head = lastDeleted;
                lastDeleted->next = nullptr;
                lastDeleted->prev = nullptr;
            }
            else
            {
                Node *temp = head;
                Node *prev = nullptr;

                while (temp != nullptr && temp->name < lastDeleted->name)
                {
                    prev = temp;
                    temp = temp->next;
                }

                if (prev == nullptr)
                {
                    lastDeleted->next = head;
                    head->prev = lastDeleted;
                    head = lastDeleted;
                    lastDeleted->prev = nullptr;
                }
                else
                {
                    lastDeleted->next = prev->next;
                    lastDeleted->prev = prev;
                    if (prev->next != nullptr)
                    {
                        prev->next->prev = lastDeleted;
                    }
                    prev->next = lastDeleted;
                }
            }
            addedContacts.push(lastDeleted);
            cout << "  Last deleted contact restored." << endl;
            SaveToFile();
        }
        else
        {
            cout << "  No contact to restore." << endl;
        }
    }

    void ShowDeletedContactsCount()
    {
        cout << "  Number of deleted contacts: " << deletedContacts.size() << endl;
    }

    void ClearContactBook()
    {
        Node *temp = head;
        stack<Node *> contactStack;
        while (temp != nullptr)
        {
            contactStack.push(temp);
            temp = temp->next;
        }

        while (!contactStack.empty())
        {
            Node *current = contactStack.top();
            contactStack.pop();
            if (current->prev != nullptr)
            {
                current->prev->next = nullptr;
            }
            else
            {
                head = nullptr;
            }
            delete current;
        }

        while (!addedContacts.empty())
        {
            addedContacts.pop();
        }
        while (!deletedContacts.empty())
        {
            deletedContacts.pop();
        }

        SaveToFile();
        cout << "  All contacts cleared." << endl;
    }

    void SaveToFile()
    {
        Node *temp = head;
        ofstream myfile("contactbook.txt");
        if (myfile.is_open())
        {
            while (temp != nullptr)
            {
                myfile << temp->name << endl;
                myfile << temp->phone_number << endl;
                temp = temp->next;
            }
            myfile.close();
        }
        else
        {
            cout << "  Unable to open file." << endl;
        }
    }

    void ReopenFromFile()
    {
        ifstream myfile("contactbook.txt");
        if (myfile.is_open() && myfile.peek() != EOF)
        {
            string line;
            while (getline(myfile, line))
            {
                string name = line;
                getline(myfile, line);
                string phone_number = line;

                Node *newNode = new Node;
                newNode->name = name;
                newNode->phone_number = phone_number;
                newNode->next = nullptr;
                newNode->prev = nullptr;

                if (head == nullptr)
                {
                    head = newNode;
                }
                else
                {
                    Node *temp = head;
                    Node *prev = nullptr;

                    while (temp != nullptr && temp->name < newNode->name)
                    {
                        prev = temp;
                        temp = temp->next;
                    }

                    if (prev == nullptr)
                    {
                        newNode->next = head;
                        head->prev = newNode;
                        head = newNode;
                    }
                    else
                    {
                        newNode->next = prev->next;
                        newNode->prev = prev;
                        if (prev->next != nullptr)
                        {
                            prev->next->prev = newNode;
                        }
                        prev->next = newNode;
                    }
                }
            }
            myfile.close();
        }
        else
        {
            cout << "  File is empty or unable to open." << endl;
        }
    }

    void SetPassword()
    {
        int password[6];
        cout << "  Set a new password (6 digits only): ";
        string input;
        cin >> input;

        while (input.length() != 6 || !isdigit(input[0]))
        {
            cout << "  Invalid input. Password must be exactly 6 digits. Try again: ";
            cin >> input;
        }

        for (int i = 0; i < 6; ++i)
        {
            password[i] = input[i] - '0';
        }

        SavePassword(password, 6);
        authenticated = true;
        cout << "  Password set successfully." << endl;
    }

    bool VerifyPassword()
    {
        ifstream passfile(passwordFile);
        if (passfile.is_open() && passfile.peek() != EOF)
        {
            int storedHashedPassword;
            passfile >> storedHashedPassword;
            passfile.close();

            int password[6];
            string input;

            while (true)
            {
                cout << "  Enter password (6 digits) or type 'cancel' to exit: ";
                cin >> input;

                if (input == "cancel")
                {
                    return false;
                }

                if (input.length() != 6 || !isdigit(input[0]))
                {
                    cout << "  Invalid input. Password must be exactly 6 digits. Try again." << endl;
                    continue;
                }

                for (int i = 0; i < 6; ++i)
                {
                    password[i] = input[i] - '0';
                }

                int enteredHashedPassword = simpleHash(password, 6);

                if (enteredHashedPassword == storedHashedPassword)
                {
                    return true;
                }
                else
                {
                    cout << "  Incorrect password. Please try again." << endl;
                }
            }
        }
        else
        {
            cout << "  No password set. Please set a password." << endl;
            SetPassword();
            return true;
        }
    }

    void ChangePassword()
    {
        if (VerifyPassword())
        {
            SetPassword();
        }
        else
        {
            cout << "  Cannot change password. Incorrect current password." << endl;
        }
    }

    void SearchContactByName(const string &name)
    {
        Node *result = SearchByName(head, name);
        if (result != nullptr)
        {
            cout << "  Contact found:" << endl;
            cout << "  Name: " << result->name << endl;
            cout << "  Phone Number: " << result->phone_number << endl;
            cout << "  Would you like to edit this contact? (Y/N): ";
            char choice;
            cin >> choice;
            if (choice == 'Y' || choice == 'y')
            {
                EditContact(result);
            }
        }
        else
        {
            cout << "  Contact not found." << endl;
        }
    }

    void SearchContactByNumber(const string &number)
    {
        Node *result = SearchByNumber(head, number);
        if (result != nullptr)
        {
            cout << "  Contact found:" << endl;
            cout << "  Name: " << result->name << endl;
            cout << "  Phone Number: " << result->phone_number << endl;
            cout << "  Would you like to edit this contact? (Y/N): ";
            char choice;
            cin >> choice;
            if (choice == 'Y' || choice == 'y')
            {
                EditContact(result);
            }
        }
        else
        {
            cout << "  Contact not found." << endl;
        }
    }

    void EditContact(Node *contact)
    {
        cout << "  Enter new name: ";
        cin.ignore();
        getline(cin, contact->name);
        cout << "  Enter new phone number: ";
        getline(cin, contact->phone_number);
        SaveToFile();
        cout << "  Contact updated successfully." << endl;
    }

    void DisplayMenu()
    {
        while (true)
        {
            cout << "=============================" << endl;
            cout << "  Contact Book Menu" << endl;
            cout << "=============================" << endl;
            cout << "  1. Add contact to queue" << endl;
            cout << "  2. Transfer contacts from queue to contact book" << endl;
            cout << "  3. Display contacts" << endl;
            cout << "  4. Delete a contact" << endl;
            cout << "  5. Undo last addition" << endl;
            cout << "  6. Undo last deletion" << endl;
            cout << "  7. Show deleted contacts count" << endl;
            cout << "  8. Clear all contacts" << endl;
            cout << "  9. Change password" << endl;
            cout << "  10. Search and edit contact" << endl;
            cout << "  11. Quit" << endl;
            cout << "=============================" << endl;
            cout << "  Enter your choice: ";
            int choice;
            cin >> choice;

            switch (choice)
            {
            case 1:
                AddToQueue();
                break;
            case 2:
                TransferFromQueue();
                break;
            case 3:
                Display();
                break;
            case 4:
                DeleteContact();
                break;
            case 5:
                UndoLastAddition();
                break;
            case 6:
                UndoLastDeletion();
                break;
            case 7:
                ShowDeletedContactsCount();
                break;
            case 8:
                ClearContactBook();
                break;
            case 9:
                if (authenticated || VerifyPassword())
                {
                    SetPassword();
                }
                break;
            case 10:
            {
                cout << "  Search contact by: " << endl;
                cout << "  1. Name" << endl;
                cout << "  2. Phone Number" << endl;
                cout << "  Enter your choice: ";
                int searchChoice;
                cin >> searchChoice;

                switch (searchChoice)
                {
                case 1:
                {
                    string name;
                    cout << "  Enter name to search: ";
                    cin.ignore();
                    getline(cin, name);
                    SearchContactByName(name);
                    break;
                }
                case 2:
                {
                    string number;
                    cout << "  Enter phone number to search: ";
                    cin.ignore();
                    getline(cin, number);
                    SearchContactByNumber(number);
                    break;
                }
                default:
                    cout << "  Invalid choice. Please try again." << endl;
                }
                break;
            }

            case 11:
                cout << "  Thank you for using Contact Book!" << endl;
                return;

            default:
                cout << "  Invalid choice. Please try again." << endl;
            }
        }
    }
};

int main()
{
    ContactBook cb;
    cb.ReopenFromFile();

    cout << "***********" << endl;
    cout << "  WELCOME TO CONTACTBOOK " << endl;
    cout << "***********" << endl;

    if (cb.VerifyPassword())
    {
        cb.DisplayMenu();
    }

    return 0;
}
