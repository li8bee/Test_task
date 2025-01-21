#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMessageBox>
#include <QSqlError>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , headerModel(new QSqlQueryModel(this))
    , dataModel(new QSqlQueryModel(this))
{
    ui->setupUi(this);

    QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");
    db.setHostName("localhost");
    db.setDatabaseName("testdb");
    db.setUserName("postgres");
    db.setPassword("admin");

    if (!db.open()) {
        QMessageBox::critical(this, "Ошибка подключения", db.lastError().text());
        return;
    }

    ui->tableViewHeaders->setModel(headerModel);
    ui->tableViewHeaders->setSelectionBehavior(QAbstractItemView::SelectRows);

    ui->tableViewData->setModel(dataModel);
    ui->tableViewData->setSelectionBehavior(QAbstractItemView::SelectRows);

    connect(ui->btnLoadData, &QPushButton::clicked, this, &MainWindow::loadData);
    connect(ui->btnLoadDataForHeader, &QPushButton::clicked, this, &MainWindow::loadDataForSelectedHeader);
    connect(ui->btnAddHeader, &QPushButton::clicked, this, &MainWindow::addHeader);
    connect(ui->btnAddData, &QPushButton::clicked, this, &MainWindow::addData);
    connect(ui->btnDeleteHeader, &QPushButton::clicked, this, &MainWindow::deleteHeader);
    connect(ui->btnDeleteData, &QPushButton::clicked, this, &MainWindow::deleteData);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadData()
{
    QSqlQuery query;
    query.prepare("SELECT id, name, description, dataCount FROM Header_tbl");

    if (!query.exec()) {
        QMessageBox::critical(this, "Ошибка запроса", query.lastError().text());
        return;
    }

    headerModel->setQuery(query);
}

void MainWindow::loadDataForSelectedHeader()
{
    QModelIndexList selectedIndexes = ui->tableViewHeaders->selectionModel()->selectedRows();

    if (selectedIndexes.isEmpty()) {
        QMessageBox::warning(this, "Предупреждение", "Выберите строку из Header_tbl.");
        return;
    }

    int headerId = headerModel->data(headerModel->index(selectedIndexes.first().row(), 0)).toInt();

    QSqlQuery query;
    query.prepare("SELECT id, data FROM Data_tbl WHERE sv_header = :headerId");
    query.bindValue(":headerId", headerId);

    if (!query.exec()) {
        QMessageBox::critical(this, "Ошибка запроса", query.lastError().text());
        return;
    }

    dataModel->setQuery(query);
}

void MainWindow::addHeader()
{
    QString name = ui->lineEditHeaderName->text();
    QString description = ui->lineEditHeaderDescription->text();

    if (name.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Имя заголовка не может быть пустым.");
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO Header_tbl (name, description, dataCount) VALUES (:name, :description, 0)");
    query.bindValue(":name", name);
    query.bindValue(":description", description);

    if (!query.exec()) {
        QMessageBox::critical(this, "Ошибка добавления", query.lastError().text());
        return;
    }

    loadData();
}

void MainWindow::addData()
{
    QModelIndexList selectedIndexes = ui->tableViewHeaders->selectionModel()->selectedRows();

    if (selectedIndexes.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Выберите строку из Header_tbl.");
        return;
    }

    int headerId = headerModel->data(headerModel->index(selectedIndexes.first().row(), 0)).toInt();
    QString data = ui->lineEditData->text();

    if (data.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Данные не могут быть пустыми.");
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO Data_tbl (data, sv_header) VALUES (:data, :headerId)");
    query.bindValue(":data", data);
    query.bindValue(":headerId", headerId);

    if (!query.exec()) {
        QMessageBox::critical(this, "Ошибка добавления", query.lastError().text());
        return;
    }

    query.prepare("UPDATE Header_tbl SET dataCount = dataCount + 1 WHERE id = :headerId");
    query.bindValue(":headerId", headerId);
    query.exec();

    loadDataForSelectedHeader();
}

void MainWindow::deleteHeader()
{
    QModelIndexList selectedIndexes = ui->tableViewHeaders->selectionModel()->selectedRows();

    if (selectedIndexes.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Выберите строку из Header_tbl.");
        return;
    }

    int headerId = headerModel->data(headerModel->index(selectedIndexes.first().row(), 0)).toInt();

    QSqlQuery query;
    query.prepare("DELETE FROM Data_tbl WHERE sv_header = :headerId");
    query.bindValue(":headerId", headerId);
    query.exec();

    query.prepare("DELETE FROM Header_tbl WHERE id = :headerId");
    query.bindValue(":headerId", headerId);

    if (!query.exec()) {
        QMessageBox::critical(this, "Ошибка удаления", query.lastError().text());
        return;
    }

    loadData();
}

void MainWindow::deleteData()
{
    QModelIndexList selectedIndexes = ui->tableViewData->selectionModel()->selectedRows();

    if (selectedIndexes.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Выберите строку из Data_tbl.");
        return;
    }

    int dataId = dataModel->data(dataModel->index(selectedIndexes.first().row(), 0)).toInt();
    int headerId = headerModel->data(headerModel->index(ui->tableViewHeaders->selectionModel()->selectedRows().first().row(), 0)).toInt();

    QSqlQuery query;
    query.prepare("DELETE FROM Data_tbl WHERE id = :dataId");
    query.bindValue(":dataId", dataId);

    if (!query.exec()) {
        QMessageBox::critical(this, "Ошибка удаления", query.lastError().text());
        return;
    }

    query.prepare("UPDATE Header_tbl SET dataCount = dataCount - 1 WHERE id = :headerId");
    query.bindValue(":headerId", headerId);
    query.exec();

    loadDataForSelectedHeader();
}
