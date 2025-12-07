#include <cs50.h>
#include <stdio.h>

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
struct Book* FindBook(string bookTitle);
bool FindIfBookExists(string bookTitle);
void CheckOutBook(string bookTitle, struct Patron *patron);
void ReturnBook(void);
void ShowBooksStatistics(void);
void ShowBooksByGenreName(void);

//Методи за потребители
bool LoadPatrons(void);
bool SavePatrons(void);
void AddPatron(void);
bool FindIfPatronExists(string patronName);
struct Patron* FindPatron(string patronName);

//Методи за резервации
bool LoadReservations(void);
bool SaveReservations(void);
int FindIfPatronHasReservedBook(int patronId);
bool FindIfBookIsReserved(int bookId);
void ReserveBook(int bookId, int patronId);
void RemoveReservaton(int bookId);

//Главни методи
bool HandleUserActions(void);
bool ShowInfo(void);
int GetMenuChoice(void);
void EndProgram(void);

int main(void)
{
    if(!LoadGenres() || !LoadBooks() || !LoadPatrons() || !LoadReservations())
    {
        printf("Error: Could not open one or more of the text files for reading.\n");
        EndProgram();
        return 1;
    }

    if(!ShowInfo())
    {
        printf("Информационният файл не е намерен.\n");
        EndProgram();
        return 1;
    }

    while(true)
    {
        bool result = HandleUserActions();
        if(!result)
        {
            EndProgram();
            return 1;
        }
    }

    if(!SaveBooks() || !SavePatrons() || !SaveReservations())
    {
        printf("Error: Could not open one or more of the text files for writing.\n");
        EndProgram();
        return 1;
    }

    return 0;
}

//Метод за главна функционалност
bool HandleUserActions(void)
{
    string patronName = get_string("Enter your name: ");
    if(!FindIfPatronExists(patronName))
    {
        AddPatron();
    }

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
        CheckOutBook(bookTitle);
    }
    else if(choice == 5)
    {
        ReturnBook();
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
    FILE *file = fopen("info.txt", "r");
    if (!file)
    {
        return false;
    }

    char line[256];
    while (fgets(line, sizeof(line), file))
    {
        printf("%s", line);
    }

    fclose(file);
    return true;
}

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
        printf("6 - Exit\n");

        choice = get_int("Your choice: ");

        if (choice >= 1 && choice <= 6)
        {
            return choice;
        }
        else
        {
            printf("Error: Invalid input! Please enter a number between 1 and 6.\n\n");
        }
    }
}

//Жанр
void ShowAllGenres(void)
{
    printf("Genres:\n");
    for(int i = 0; i < genresCount; i++)
    {
        printf("%s\n", genres[i].GenreName);
    }
}

int GetGenreId(string genreName)
{
    int genreId = 0;
    for(int i = 0; i < genresCount; i++)
    {
        if(genres[i].GenreName == genreName)
        {
            genreId = genres[i].GenreId;
            break;
        }
    }
    return genreId;
}

//Книга
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

    SaveBooks();

    printf("Book \"%s\" added successfully!\n", newBook.BookTitle);
}

struct Book* FindBook(string bookTitle)
{
    if(!FindIfBookExists(bookTitle))
    {
        printf("There is no Book with the given name\n");
        return NULL;
    }

    for(int i = 0; i < booksCount; i++)
    {
        if(bookTitle == books[i].BookTitle)
        {
            return &books[i];
        }
    }

    return NULL;
}

bool FindIfBookExists(string bookTitle)
{
    for(int i = 0; i < booksCount; i++)
    {
        if(bookTitle == books[i].BookTitle)
        {
            return true;
        }
    }
    return false;
}

void CheckOutBook(string bookTitle, struct Patron *patron)
{
    if(!patron->HasCheckedOutBook)
    {
        printf("First you must return the book you have borrowed and then you can borrow other book\n");
        return;
    }

    struct Book *book = FindBook(bookTitle);
    if(bool == NULL)
    {
        return;
    }

    if(book->IsCheckedOut)
    {
        printf("The book is already borrowed\n");
        if(patron->CountOfCheckouts == 3)
        {
            //Шанс за резервиране
        }
    }
}


//Зареждане на данни
bool LoadGenres(void)
{
    FILE *file = fopen("genres.txt", "r");
    if (!file) return false;

    genresCount = 0;
    while (fscanf(file, "%d;%[^\n]\n"
        , &genres[genresCount].Id
        , genres[genresCount].GenreName) == 2)
    {
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
    while (fscanf(file, "%d;%[^;];%[^;];%d;%d;%d\n",
                  &books[booksCount].Id,
                  books[booksCount].BookTitle,
                  books[booksCount].Author,
                  &books[booksCount].GenreId,
                  (int*)&books[booksCount].IsCheckedOut,
                  &books[booksCount].CountOfCheckouts) == 6)
    {
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
    while (fscanf(file, "%d;%[^;];%d;%d\n",
                  &patrons[patronsCount].Id,
                  patrons[patronsCount].PatronName,
                  (int*)&patrons[patronsCount].HasCheckedOutBook,
                  &patrons[patronsCount].ReturnedBooksCount,
                  &patrons[patronsCount].BorrowedBookId) == 5)
    {
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
    while (fscanf(file, "%d;%d\n",
                  &reservations[reservationsCount].BookId,
                  &reservations[reservationsCount].PatronId) == 2)
    {
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
    {
        return false;
    }

    for (int i = 0; i < booksCount; i++)
    {
        fprintf(file, "%d;%s;%s;%d;%d;%d\n",
                books[i].Id,
                books[i].BookTitle,
                books[i].Author,
                books[i].GenreId,
                books[i].IsCheckedOut,
                books[i].CountOfCheckouts);
    }

    fclose(file);
    return true;
}

bool SavePatrons(void)
{
    FILE *file = fopen("patrons.txt", "w");
    if (!file)
    {
        return false;
    }

    for (int i = 0; i < patronsCount; i++)
    {
        fprintf(file, "%d;%s;%d;%d\n",
                patrons[i].Id,
                patrons[i].PatronName,
                patrons[i].HasCheckedOutBook,
                patrons[i].ReturnedBooksCount,
                patrons[i].BorrowedBookId);
    }

    fclose(file);
    return true;
}

bool SaveReservations(void)
{
    FILE *file = fopen("reservations.txt", "w");
    if (!file)
    {
        return false;
    }

    for (int i = 0; i < reservationsCount; i++)
    {
        fprintf(file, "%d;%d\n",
                reservations[i].BookId,
                reservations[i].PatronId);
    }

    fclose(file);
    return true;
}
