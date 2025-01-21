#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSqlDatabase>
#include <QSqlQueryModel>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QItemSelectionModel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , headerModel(new QSqlQueryModel(this))  // Инициализация модели для SQL-запросов Header_tbl
    , dataModel(new QSqlQueryModel(this))    // Инициализация модели для SQL-запросов Data_tbl
{
    ui->setupUi(this);

    // Подключение к базе данных
    QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");
    db.setHostName("localhost");
    db.setDatabaseName("testdb");
    db.setUserName("postgres");
    db.setPassword("admin");

    if (!db.open()) {
        QMessageBox::critical(this, "Ошибка подключения", db.lastError().text());
        return;
    }

    // Настройка модели для Header_tbl
    ui->tableViewHeaders->setModel(headerModel);
    ui->tableViewHeaders->setEditTriggers(QAbstractItemView::NoEditTriggers);  // Отключаем редактирование
    ui->tableViewHeaders->setSelectionBehavior(QAbstractItemView::SelectRows);  // Выделение строк
    ui->tableViewHeaders->setSelectionMode(QAbstractItemView::SingleSelection);  // Выбор одной строки

    // Настройка модели для Data_tbl
    ui->tableViewData->setModel(dataModel);
    ui->tableViewData->setEditTriggers(QAbstractItemView::NoEditTriggers);  // Отключаем редактирование
    ui->tableViewData->setSelectionBehavior(QAbstractItemView::SelectRows);  // Выделение строк
    ui->tableViewData->setSelectionMode(QAbstractItemView::SingleSelection);  // Выбор одной строки

    // Подключаем кнопку загрузки данных
    connect(ui->btnLoadData, &QPushButton::clicked, this, &MainWindow::loadData);

    // Подключаем кнопку для загрузки данных из Data_tbl для выбранного заголовка
    connect(ui->btnLoadDataForHeader, &QPushButton::clicked, this, &MainWindow::loadDataForSelectedHeader);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadData()
{
    // Загружаем все данные из Header_tbl при старте
    QSqlQuery query;
    query.prepare("SELECT id, name, description, dataCount FROM Header_tbl");

    if (!query.exec()) {
        QMessageBox::critical(this, "Ошибка запроса", query.lastError().text());
        return;
    }

    // Передаем результат запроса в модель
    headerModel->setQuery(query);

    // Устанавливаем заголовки столбцов
    headerModel->setHeaderData(0, Qt::Horizontal, "ID");
    headerModel->setHeaderData(1, Qt::Horizontal, "Name");
    headerModel->setHeaderData(2, Qt::Horizontal, "Description");
    headerModel->setHeaderData(3, Qt::Horizontal, "Data Count");
}

void MainWindow::loadDataForSelectedHeader()
{
    // Получаем индекс выбранной строки в tableViewHeaders
    QModelIndexList selectedIndexes = ui->tableViewHeaders->selectionModel()->selectedRows();

    if (selectedIndexes.isEmpty()) {
        QMessageBox::warning(this, "Предупреждение", "Пожалуйста, выберите строку из Header_tbl.");
        return;
    }

    int selectedRow = selectedIndexes.first().row();
    int headerId = headerModel->data(headerModel->index(selectedRow, 0)).toInt(); // Получаем ID выбранного заголовка

    // Теперь делаем запрос для получения данных из Data_tbl, связанных с этим заголовком
    QSqlQuery query;
    query.prepare("SELECT id, data FROM Data_tbl WHERE sv_header = :headerId");
    query.bindValue(":headerId", headerId);

    if (!query.exec()) {
        QMessageBox::critical(this, "Ошибка запроса", query.lastError().text());
        return;
    }

    // Передаем результат запроса в модель для Data_tbl
    dataModel->setQuery(query);

    // Устанавливаем заголовки столбцов для Data_tbl
    dataModel->setHeaderData(0, Qt::Horizontal, "ID");
    dataModel->setHeaderData(1, Qt::Horizontal, "Data");
}
