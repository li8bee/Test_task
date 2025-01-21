#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QTableView>
#include <QDebug>
#include <QMessageBox>
#include <QSqlError>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , model(new QSqlTableModel(this))
{
    ui->setupUi(this);

    // Подключаемся к базе данных PostgreSQL
    QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");
    db.setHostName("localhost");      // Адрес сервера
    db.setDatabaseName("testdb");     // Имя базы данных
    db.setUserName("postgres");       // Имя пользователя
    db.setPassword("password");       // Пароль

    if (!db.open()) {
        QMessageBox::critical(this, "Ошибка подключения", db.lastError().text());
        return;
    }

    qDebug() << "Соединение с базой данных успешно!";

    // Настраиваем модель для таблицы Header_tbl
    model->setTable("Header_tbl");
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->select();

    // Устанавливаем заголовки для отображения
    model->setHeaderData(0, Qt::Horizontal, "ID");
    model->setHeaderData(1, Qt::Horizontal, "Name");
    model->setHeaderData(2, Qt::Horizontal, "Description");
    model->setHeaderData(3, Qt::Horizontal, "Data Count");

    // Создаём виджет QTableView для отображения данных
    QTableView *tableView = new QTableView(this);
    tableView->setModel(model);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers); // Данные только для чтения
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows); // Выбор строки целиком
    tableView->setSelectionMode(QAbstractItemView::SingleSelection); // Одна строка за раз

    // Устанавливаем виджет в центральную часть окна
    setCentralWidget(tableView);
}

MainWindow::~MainWindow()
{
    delete ui;
}
