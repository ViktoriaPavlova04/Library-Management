#include <cs50.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Дефиниции
#define MAX_BOOKS 1000
#define MAX_PATRONS 1000
#define MAX_RESERVATIONS 1000
#define MAX_GENRES 11

//Структури
struct Genre
{
    int Id;
    string GenreName;
};

struct Book
{
    int Id;
    string BookTitle;
    string Author;
    int GenreId;
    bool IsCheckedOut;
    int CountOfCheckouts;
};

struct Patron
{
    int Id;
    string PatronName;
    bool HasCheckedOutBook;
    int ReturnedBooksCount;
    int BorrowedBookId;
};

struct Reservation
{
    int BookId;
    int PatronId;
};

//Глобални променливи
struct Genre genres[MAX_GENRES];
int genresCount = 0;

struct Book books[MAX_BOOKS];
int booksCount = 0;

struct Patron patrons[MAX_PATRONS];
int patronsCount = 0;

struct Reservation reservations[MAX_RESERVATIONS];
int reservationsCount = 0;

//Дефиниране на методите
//Методи за жанр
bool LoadGenres(void);
void ShowAllGenres(void);
int GetGenreId(string genreName);

//Методи за книги
bool LoadBooks(void);
bool SaveBooks(void);
void AddBook(void);
void ShowBooks(void);
struct Book* FindBook(string bookTitle);
string GetBookTitle(int bookId);
bool FindIfBookExists(string bookTitle);
void CheckOutBook(string bookTitle, struct Patron *patron);
void ReturnBook(string patronName);
void ShowBookTitles(void);
int CompareBooksByCheckouts(const void *a, const void *b);
void ShowBooksStatistics(void);
void ShowBooksByGenreName(void);
bool IsBookChekedOut(int bookId);

//Методи за потребители
bool LoadPatrons(void);
bool SavePatrons(void);
string AddPatron(void);
bool FindIfPatronExists(string patronName);
struct Patron* FindPatron(string patronName);

//Методи за резервации
bool LoadReservations(void);
bool SaveReservations(struct Reservation reservationsArray[], int count);
int FindIfPatronHasReservedBook(int patronId);
bool FindIfPatronAlreadyHasMadeReservationForABook(int patronId, int bookId);
bool FindIfIsPatronTurnToBorrowReservedBook(int bookId, int patronId);
void ReserveBook(int bookId, int patronId);
void RemoveReservation(int bookId, int patronId);

//Главни методи
bool HandleUserActions(void);
bool ShowInfo(void);
int GetMenuChoice(void);
void EndProgram(void);

//Зачистване на паметта
void FreeGenres(struct Genre genresArray[], int count);
void FreeBooks(struct Book booksArray[], int count);
void FreePatrons(struct Patron patronsArray[], int count);

//Помощен метод за копиране на string от текстовите документи
char* my_strdup(const char* s);

//Основен метод
int main(void)
{
    //Зареждаме всички данни от тесктовите документи
    if(!LoadGenres() || !LoadBooks() || !LoadPatrons() || !LoadReservations())
    {
        printf("Error: Could not open one or more of the text files for reading.\n");
        EndProgram();
        return 1;
    }
    //Зареждаме информационен текст
    if(!ShowInfo())
    {
        printf("The information file cannot be found.\n");
        EndProgram();
        return 1;
    }

    //Цикъл, който преизпълнява функциите си при всяка успешна операция
    while(true)
    {
        bool result = HandleUserActions();
        if(!result)
        {
            break;
        }
    }

    //Запаметяваме данните в текстовите документи
    if(!SaveBooks() || !SavePatrons() || !SaveReservations(reservations, reservationsCount))
    {
        printf("Error: Could not open one or more of the text files for writing.\n");
        EndProgram();
        return 1;
    }

    //Зачистваме паметта (единствено масивите, който в себе си имат string елементи)
    FreeGenres(genres, genresCount);
    FreeBooks(books, booksCount);
    FreePatrons(patrons, patronsCount);

    return 0;
}

//Метод за главна функционалност
bool HandleUserActions(void)
{
    //Взимаме името на потребителя
    bool ifPatronAlreadyExists = true;
    string patronName = get_string("Enter your name: ");
    //Ако не го намираме в системата, добавяме нов потребител
    if(!FindIfPatronExists(patronName))
    {
        patronName = AddPatron();
        if(strlen(patronName) == 0)
        {
            printf("Something went wrong. \n");
            return false;
        }
        ifPatronAlreadyExists = false;
    }

    //Ако е намерен в системата проверяваме дали има направена резервация
    //Ако има, негов ред е да я вземе и в момента не си е взел книга, я взима.
    if(ifPatronAlreadyExists)
    {
        struct Patron *patron = FindPatron(patronName);
        int bookId = FindIfPatronHasReservedBook(patron->Id);
        if(bookId != 0)
        {
            if(!IsBookChekedOut(bookId) && FindIfIsPatronTurnToBorrowReservedBook(bookId, patron->Id))
            {
                printf("You can check out the book you have reserved.\n");
                char result = get_char("Do you want to check out the reserved book? (y/n): \n");

                if(result == 'y')
                {
                    string bookTitle = GetBookTitle(bookId);
                    CheckOutBook(bookTitle, patron);
                    RemoveReservation(bookId, patron->Id);
                }
            }
        }
    }

    //Разбираме каква операция иска да извърши потребителят
    int choice = GetMenuChoice();

    if(choice == 1)
    {
        AddBook();
    }
    else if(choice == 2)
    {
        ShowBooks();
    }
    else if(choice == 3)
    {
        ShowBooksByGenreName();
    }
    else if(choice == 4)
    {
        string bookTitle = get_string("Enter the title of the book you want to check out: ");
        struct Patron *patron = FindPatron(patronName);
        CheckOutBook(bookTitle, patron);
    }
    else if(choice == 5)
    {
        ReturnBook(patronName);
    }
    else if(choice == 6)
    {
        ShowBooksStatistics();
    }
    else
    {
        return false;
    }

    return true;
}

//Показване на базова информация (при всяко стартиране на програмата)
bool ShowInfo(void)
{
    //Отваряме файла в режим четене
    FILE *file = fopen("info.txt", "r");
    if (!file)
    {
        return false;
    }

    //Ред по ред прочитаме файла
    char line[256];
    while (fgets(line, sizeof(line), file))
    {
        printf("%s", line);
    }

    //Затваряме го
    fclose(file);
    return true;
}

//Метод, с който се взима желаната операция от потребител
int GetMenuChoice(void)
{
    int choice;

    while (true)
    {
        printf("Select the number of the service you want:\n");
        printf("1 - Add a book\n");
        printf("2 - View all books\n");
        printf("3 - View books by a specific genre\n");
        printf("4 - Check out a book\n");
        printf("5 - Return book\n");
        printf("6 - View books statistics\n");
        printf("7 - Exit\n");

        choice = get_int("Your choice: \n");

        if (choice >= 1 && choice <= 7)
        {
            return choice;
        }
        else
        {
            printf("Error: Invalid input! Please enter a number between 1 and 7.\n\n");
        }
    }
}

void EndProgram(void)
{
    printf("Exit of Library App \n");
}

//Жанр

//Показваме всички жанрове
void ShowAllGenres(void)
{
    printf("Genres:\n");
    for(int i = 0; i < genresCount; i++)
    {
        printf("%s\n", genres[i].GenreName);
    }
}

//Взимаме идентификатор на жанр по негово име
int GetGenreId(string genreName)
{
    int genreId = 0;
    for(int i = 0; i < genresCount; i++)
    {
        if(strcmp(genres[i].GenreName, genreName) == 0)
        {
            genreId = genres[i].Id;
            break;
        }
    }
    return genreId;
}

//Книга

//Добавяме книга
void AddBook(void)
{
    if (booksCount >= MAX_BOOKS)
    {
        printf("Error: Maximum number of books reached!\n");
        return;
    }

    struct Book newBook;
    newBook.Id = booksCount + 1;
    string title = get_string("Enter the book title: ");

    if(FindIfBookExists(title))
    {
        printf("There is already a book with the same title in the library\n");
        return;
    }
    snprintf(newBook.BookTitle, sizeof(newBook.BookTitle), "%s", title);

    string author = get_string("Enter the author: ");
    snprintf(newBook.Author, sizeof(newBook.Author), "%s", author);

    ShowAllGenres();
    string genreName = get_string("Enter genre Name: ");
    int genreId = GetGenreId(genreName);

    if(genreId == 0)
    {
        printf("There is no Genre with the given name\n");
        return;
    }

    newBook.GenreId = genreId;
    newBook.IsCheckedOut = false;
    newBook.CountOfCheckouts = 0;

    books[booksCount] = newBook;
    booksCount++;

    if(!SaveBooks())
    {
        printf("Something went wrong. \n");
        return;
    }

    printf("Book \"%s\" added successfully!\n", newBook.BookTitle);
}

//Показване на всички книги
void ShowBooks(void)
{
    for(int i = 0; i < booksCount; i++)
    {
        printf("Book Title: %s - Author: %s \n", books[i].BookTitle, books[i].Author);
    }
}

//Намираме книга по нейното име
struct Book* FindBook(string bookTitle)
{
    if(!FindIfBookExists(bookTitle))
    {
        printf("There is no Book with the given name\n");
        return NULL;
    }

    for(int i = 0; i < booksCount; i++)
    {
        if(strcmp(bookTitle, books[i].BookTitle) == 0)
        {
            return &books[i];
        }
    }

    return NULL;
}

//Намираме име на книга по идентификатор
string GetBookTitle(int bookId)
{
    for(int i = 0; i < booksCount; i++)
    {
        if(bookId == books[i].Id)
        {
            return books[i].BookTitle;
        }
    }

    return "";
}

//Намираме дали дадено заглавие на книга съществува
bool FindIfBookExists(string bookTitle)
{
    for(int i = 0; i < booksCount; i++)
    {
        if(strcmp(bookTitle, books[i].BookTitle) == 0)
        {
            return true;
        }
    }
    return false;
}

//Метод за взимане на книга
void CheckOutBook(string bookTitle, struct Patron *patron)
{
    //Трябва потребителят да е върнал книга преди да вземе нова
    if(patron->HasCheckedOutBook)
    {
        printf("First you must return the book you have borrowed and then you can borrow other book\n");
        return;
    }

    //Намираме желаната книга
    struct Book *book = FindBook(bookTitle);
    if(book == NULL)
    {
        return;
    }

    //Ако книгата е вече взета, проверяваме дали потребителят може да я резервира
    if(book->IsCheckedOut)
    {
        printf("The book is already borrowed\n");
        if(patron->ReturnedBooksCount >= 3)
        {
            printf("Since you have returned 3 books so far, you have the right to reserve a book of your choice.\n");

            ShowBookTitles();

            char result = get_char("Do you want to reserve a book? (y/n): \n");
            if(result == 'y')
            {
                while(true)
                {
                    int choice = get_int("Select the number of your choice: \n");

                    if (choice >= 1 && choice <= booksCount)
                    {
                        if(FindIfPatronAlreadyHasMadeReservationForABook(patron->Id, choice))
                        {
                            printf("You have already reserved this book. We will notify you when it is your turn to pick it up.\n");
                        }
                        else
                        {
                            ReserveBook(choice, patron->Id);
                        }
                    }
                }
                patron->ReturnedBooksCount = 0;
                return;
            }
        }
    }

    //Взима книгата
    books[(book->Id - 1)].IsCheckedOut = true;
    books[(book->Id - 1)].CountOfCheckouts++;
    if(!SaveBooks())
    {
        printf("Something went wrong. \n");
        return;
    }

    patrons[(patron->Id - 1)].HasCheckedOutBook = true;
    patrons[(patron->Id - 1)].BorrowedBookId = book->Id;
    if(!SavePatrons())
    {
        printf("Something went wrong. \n");
        return;
    }

    printf("You have successfully checked out the book \"%s\". \n", book->BookTitle);
}

//Метод за връщане на книга
void ReturnBook(string patronName)
{
    struct Patron *patron = FindPatron(patronName);
    if(patron == NULL)
    {
        return;
    }

    if(!patron->HasCheckedOutBook)
    {
        printf("You have not checked out a book to returned it.\n");
    }
    string bookTitle = GetBookTitle(patron->BorrowedBookId);

    books[(patron->BorrowedBookId - 1)].IsCheckedOut = false;
    if(!SaveBooks())
    {
        printf("Something went wrong. \n");
        return;
    }

    patrons[(patron->Id - 1)].HasCheckedOutBook = false;
    patrons[(patron->Id - 1)].BorrowedBookId = 0;
    if(!SavePatrons())
    {
        printf("Something went wrong. \n");
        return;
    }

    printf("You have successfully returned the book \"%s\". \n", bookTitle);
}

//Показваме само заглавията на книгите
void ShowBookTitles(void)
{
    for(int i = 0; i < booksCount; i++)
    {
        printf("%d: %s", (i+1), books[i].BookTitle);
    }
}

//Метод, който се използва във функцията qsort
//По този начин книгите се сравняват две по две и програмата преценя как да ги нареди в списъка
int CompareBooksByCheckouts(const void *a, const void *b)
{
    const struct Book *bookA = (const struct Book *)a;
    const struct Book *bookB = (const struct Book *)b;

    if (bookA->CountOfCheckouts < bookB->CountOfCheckouts) return 1;
    if (bookA->CountOfCheckouts > bookB->CountOfCheckouts) return -1;

    return 0;
}

//Показваме статистика - книги, наредени по броя на взимане
void ShowBooksStatistics(void)
{
    struct Book sortedBooks[MAX_BOOKS];

    for(int i = 0; i < booksCount; i++)
    {
        sortedBooks[i] = books[i];
    }

    qsort(sortedBooks, booksCount, sizeof(struct Book), CompareBooksByCheckouts);

    for (int i = 0; i < booksCount; i++)
    {
        printf("%s (%d checkouts)\n", sortedBooks[i].BookTitle,sortedBooks[i].CountOfCheckouts);
    }
}

//Показва всички книги по даден жанр
void ShowBooksByGenreName(void)
{
    ShowAllGenres();
    string genreName = get_string("Enter genre Name: ");
    int genreId = GetGenreId(genreName);

    if(genreId == 0)
    {
        printf("There is no Genre with the given name\n");
        return;
    }

    for(int i = 0; i < booksCount; i++)
    {
        if(books[i].GenreId == genreId)
        {
            printf("Book Title: %s - Author: %s \n", books[i].BookTitle, books[i].Author);
        }
    }
}

//Проверява дали книгата с даден идентификатор е взета
bool IsBookChekedOut(int bookId)
{
    for(int i = 0; i < booksCount; i++)
    {
        if(books[i].Id == bookId && books[i].IsCheckedOut == true)
        {
            return true;
        }
    }

    return false;
}

//Потребители

//Метод за добавяне на потребители
string AddPatron(void)
{
    if (patronsCount >= MAX_PATRONS)
    {
        printf("Error: Maximum number of patrons reached!\n");
        return "";
    }

    struct Patron newPatron;
    newPatron.Id = patronsCount + 1;

    string name = get_string("Enter the patron name: ");

    while(true)
    {
        if(!FindIfPatronExists(name))
        {
            break;
        }
        printf("There is a patron with the same name.");
        name = get_string("Enter the patron name: ");
    }

    //Тъй като string е масив за по-безопасно копирам стойността на name в newPatron.PatronName
    snprintf(newPatron.PatronName, sizeof(newPatron.PatronName), "%s", name);
    newPatron.HasCheckedOutBook = false;
    newPatron.ReturnedBooksCount = 0;
    newPatron.BorrowedBookId = 0;

    patrons[patronsCount] = newPatron;
    patronsCount++;

    if(!SavePatrons())
    {
        printf("Something went wrong. \n");
        return "";
    }

    printf("Patron %s added successfully!\n", name);
    return name;
}

//Намираме дали потребител със същото име съществува
bool FindIfPatronExists(string patronName)
{
    for(int i = 0; i < patronsCount; i++)
    {
        if(strcmp(patrons[i].PatronName, patronName) == 0)
        {
            return true;
        }
    }
    return false;
}

//Намираме даден потребител по име
struct Patron* FindPatron(string patronName)
{
    if(!FindIfPatronExists(patronName))
    {
        printf("There is no Patron with the given name\n");
        return NULL;
    }

    for(int i = 0; i < patronsCount; i++)
    {
        if(strcmp(patronName, patrons[i].PatronName) == 0)
        {
            return &patrons[i];
        }
    }

    return NULL;
}

//Резервация

//Намираме дали потребител с даден идентификатор е резервирал книга
int FindIfPatronHasReservedBook(int patronId)
{
    for(int i = 0; i < reservationsCount; i++)
    {
        if(reservations[i].PatronId == patronId)
        {
            return reservations[i].BookId;
        }
    }
    return 0;
}

//Намираме дали потребител вече направил резервация за тази книга
bool FindIfPatronAlreadyHasMadeReservationForABook(int patronId, int bookId)
{
    for(int i = 0; i < reservationsCount; i++)
    {
        if(reservations[i].PatronId == patronId && reservations[i].BookId == bookId)
        {
            return true;
        }
    }

    return false;
}

//Проверяваме дали е ред на дадения потребител да вземе резервираната книга
bool FindIfIsPatronTurnToBorrowReservedBook(int bookId, int patronId)
{
    for(int i = 0; i < reservationsCount; i++)
    {
        if(reservations[i].PatronId != patronId && reservations[i].BookId == bookId)
        {
            return false;
        }
        else if(reservations[i].PatronId == patronId && reservations[i].BookId == bookId)
        {
            return true;
        }
    }

    return false;
}

//Резервиране на книга
void ReserveBook(int bookId, int patronId)
{
    if (reservationsCount >= MAX_RESERVATIONS)
    {
        printf("Error: Maximum number of reservations reached!\n");
        return;
    }

    struct Reservation newReservation;
    newReservation.BookId = bookId;
    newReservation.PatronId = patronId;

    reservations[reservationsCount] = newReservation;
    reservationsCount++;

    if(!SaveReservations(reservations, reservationsCount))
    {
        printf("Something went wrong. \n");
        return;
    }

    printf("The book is reserved. \n");
}

//Премахваме резервация
void RemoveReservation(int bookId, int patronId)
{
    struct Reservation reservationsToStay[MAX_RESERVATIONS];
    int count = 0;

    for(int i = 0; i < reservationsCount; i++)
    {
        if(reservations[i].PatronId == patronId && reservations[i].BookId == bookId)
        {
            continue;
        }

        reservationsToStay[count].PatronId = reservations[i].PatronId;
        reservationsToStay[count].BookId = reservations[i].BookId;
        count++;
    }

    if(!SaveReservations(reservationsToStay, count))
    {
        printf("Something went wrong. \n");
        return;
    }

    reservationsCount = 0;
    memset(reservations, 0, sizeof(reservations));

    LoadReservations();
}

//Зареждане на данни
bool LoadGenres(void)
{
    FILE *file = fopen("genres.txt", "r");
    if (!file) return false;

    genresCount = 0;
    char line[256];

    while (fgets(line, sizeof(line), file) && genresCount < MAX_GENRES)
    {
        line[strcspn(line, "\n")] = 0;

        if (strlen(line) == 0) continue;

        char *id = strtok(line, "|");
        char *name = strtok(NULL, "|");

        if (!id || !name) continue;

        genres[genresCount].Id = atoi(id);
        genres[genresCount].GenreName = my_strdup(name);

        genresCount++;
    }

    fclose(file);
    return true;
}

bool LoadBooks(void)
{
    FILE *file = fopen("books.txt", "r");
    if (!file) return false;

    booksCount = 0;
    char line[512];

    while (fgets(line, sizeof(line), file) && booksCount < MAX_BOOKS)
    {
        line[strcspn(line, "\n")] = 0;

        if (strlen(line) == 0) continue;

        char *id = strtok(line, "|");
        char *title = strtok(NULL, "|");
        char *author = strtok(NULL, "|");
        char *genreId = strtok(NULL, "|");
        char *checked = strtok(NULL, "|");
        char *checkouts = strtok(NULL, "|");

        if (!id || !title || !author || !genreId || !checked || !checkouts)
            continue;

        books[booksCount].Id = atoi(id);
        books[booksCount].BookTitle = my_strdup(title);
        books[booksCount].Author = my_strdup(author);
        books[booksCount].GenreId = atoi(genreId);
        books[booksCount].IsCheckedOut = atoi(checked);
        books[booksCount].CountOfCheckouts = atoi(checkouts);

        booksCount++;
    }

    fclose(file);
    return true;
}

bool LoadPatrons(void)
{
    FILE *file = fopen("patrons.txt", "r");
    if (!file) return false;

    patronsCount = 0;
    char line[256];

    while (fgets(line, sizeof(line), file) && patronsCount < MAX_PATRONS)
    {
        line[strcspn(line, "\n")] = 0;

        if (strlen(line) == 0) continue;

        char *id = strtok(line, "|");
        char *name = strtok(NULL, "|");
        char *hasBook = strtok(NULL, "|");
        char *returned = strtok(NULL, "|");
        char *borrowed = strtok(NULL, "|");

        if (!id || !name || !hasBook || !returned || !borrowed)
            continue;

        patrons[patronsCount].Id = atoi(id);
        patrons[patronsCount].PatronName = my_strdup(name);
        patrons[patronsCount].HasCheckedOutBook = atoi(hasBook);
        patrons[patronsCount].ReturnedBooksCount = atoi(returned);
        patrons[patronsCount].BorrowedBookId = atoi(borrowed);

        patronsCount++;
    }

    fclose(file);
    return true;
}

bool LoadReservations(void)
{
    FILE *file = fopen("reservations.txt", "r");
    if (!file) return false;

    reservationsCount = 0;
    char line[256];

    while (fgets(line, sizeof(line), file) && reservationsCount < MAX_RESERVATIONS)
    {
        line[strcspn(line, "\n")] = 0;

        if (strlen(line) == 0) continue;

        char *bookId = strtok(line, "|");
        char *patronId = strtok(NULL, "|");

        if (!bookId || !patronId) continue;

        reservations[reservationsCount].BookId = atoi(bookId);
        reservations[reservationsCount].PatronId = atoi(patronId);
        reservationsCount++;
    }

    fclose(file);
    return true;
}


//Запазване на данните
bool SaveBooks(void)
{
    FILE *file = fopen("books.txt", "w");
    if (!file)
        return false;

    for (int i = 0; i < booksCount; i++)
    {
        fprintf(file, "%d|%s|%s|%d|%d|%d\n",
                books[i].Id,
                books[i].BookTitle,
                books[i].Author,
                books[i].GenreId,
                books[i].IsCheckedOut ? 1 : 0,
                books[i].CountOfCheckouts);
    }

    fclose(file);
    return true;
}

bool SavePatrons(void)
{
    FILE *file = fopen("patrons.txt", "w");
    if (!file)
        return false;

    for (int i = 0; i < patronsCount; i++)
    {
        fprintf(file, "%d|%s|%d|%d|%d\n",
                patrons[i].Id,
                patrons[i].PatronName,
                patrons[i].HasCheckedOutBook ? 1 : 0,
                patrons[i].ReturnedBooksCount,
                patrons[i].BorrowedBookId);
    }

    fclose(file);
    return true;
}

bool SaveReservations(struct Reservation reservationsArray[], int count)
{
    FILE *file = fopen("reservations.txt", "w");
    if (!file)
        return false;

    for (int i = 0; i < reservationsCount; i++)
    {
        fprintf(file, "%d|%d\n",
                reservations[i].BookId,
                reservations[i].PatronId);
    }

    fclose(file);
    return true;
}

//Зачистване на паметта
void FreeGenres(struct Genre genresArray[], int count)
{
    for (int i = 0; i < count; i++)
    {
        free(genresArray[i].GenreName);
    }
}

void FreeBooks(struct Book booksArray[], int count)
{
    for (int i = 0; i < count; i++)
    {
        free(booksArray[i].BookTitle);
        free(booksArray[i].Author);
    }
}

void FreePatrons(struct Patron patronsArray[], int count)
{
    for (int i = 0; i < count; i++)
    {
        free(patronsArray[i].PatronName);
    }
}

char* my_strdup(const char* s)
{
    if (s == NULL) {
        return NULL;
    }

    char* copy = malloc(strlen(s) + 1);

    if (copy == NULL){
        return NULL;
    }

    strcpy(copy, s);
    return copy;
}
