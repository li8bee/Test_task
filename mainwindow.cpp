#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMessageBox>
#include <QSqlError>

#include <QFileDialog>
#include <QFile>
#include <QTextStream>

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
    connect(ui->btnExportToCSV, &QPushButton::clicked, this, &MainWindow::exportHeaderToCSV);
    connect(ui->btnImportFromCSV, &QPushButton::clicked, this, &MainWindow::importHeaderFromCSV);
    connect(ui->btnExportAllToCSV, &QPushButton::clicked, this, &MainWindow::exportAllToCSV);
}

MainWindow::~MainWindow()
{
    delete ui;
}

/************************************************************
 *                                                          *
 *             Отображение таблицы                          *
 *                                                          *
 ************************************************************/

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

/************************************************************
 *                                                          *
 *             Добавление строк Header / Data               *
 *                                                          *
 ************************************************************/

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

/************************************************************
 *                                                          *
 *             Удаление строк Header / Data                 *
 *                                                          *
 ************************************************************/


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



/************************************************************
 *                                                          *
 *             Импорт / Экспорт данных в .CSV               *
 *                                                          *
 ************************************************************/

void MainWindow::exportHeaderToCSV()
{
    // Проверяем, выбран ли заголовок
    QModelIndexList selectedIndexes = ui->tableViewHeaders->selectionModel()->selectedRows();
    if (selectedIndexes.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Выберите заголовок для экспорта.");
        return;
    }

    // Получаем данные выбранного заголовка
    int selectedRow = selectedIndexes.first().row();
    int headerId = headerModel->data(headerModel->index(selectedRow, 0)).toInt();
    QString headerName = headerModel->data(headerModel->index(selectedRow, 1)).toString();
    QString headerDescription = headerModel->data(headerModel->index(selectedRow, 2)).toString();

    // Выбор файла для сохранения
    QString filePath = QFileDialog::getSaveFileName(this, "Сохранить файл", headerName + ".csv", "CSV Files (*.csv)");
    if (filePath.isEmpty()) {
        return;
    }

    // Открываем файл для записи
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Ошибка", "Не удалось открыть файл для записи.");
        return;
    }

    QTextStream out(&file);

    // Записываем данные заголовка
    out << headerName << ";" << headerDescription << "\n";

    // Получаем данные, связанные с этим заголовком, из Data_tbl
    QSqlQuery query;
    query.prepare("SELECT data FROM Data_tbl WHERE sv_header = :headerId");
    query.bindValue(":headerId", headerId);

    if (!query.exec()) {
        QMessageBox::critical(this, "Ошибка", "Не удалось выполнить запрос к базе данных.");
        file.close();
        return;
    }

    // Записываем данные в файл
    while (query.next()) {
        out << query.value(0).toString() << "\n";
    }

    file.close();
    QMessageBox::information(this, "Успех", "Данные успешно экспортированы.");
}

void MainWindow::importHeaderFromCSV()
{
    // Выбор файла для импорта
    QString filePath = QFileDialog::getOpenFileName(this, "Открыть файл", "", "CSV Files (*.csv)");
    if (filePath.isEmpty()) {
        return;
    }

    // Открываем файл для чтения
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Ошибка", "Не удалось открыть файл для чтения.");
        return;
    }

    QTextStream in(&file);
    QSqlQuery query;
    int importedHeaders = 0;
    int importedData = 0;

    QString headerName;
    QString headerDescription;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();

        // Пропускаем пустые строки
        if (line.isEmpty()) {
            continue;
        }

        // Если строка содержит ";", это заголовок
        if (line.contains(";")) {
            QStringList headerParts = line.split(";");
            if (headerParts.size() < 2) {
                QMessageBox::warning(this, "Ошибка", "Строка заголовка должна содержать Name и Description, разделённые точкой с запятой.");
                file.close();
                return;
            }

            // Завершаем добавление данных для предыдущего заголовка, если он был
            if (!headerName.isEmpty()) {
                query.prepare("UPDATE Header_tbl SET dataCount = (SELECT COUNT(*) FROM Data_tbl WHERE sv_header = :headerId) WHERE id = :headerId");
                query.bindValue(":headerId", importedHeaders);
                query.exec();
            }

            // Обрабатываем новый заголовок
            headerName = headerParts[0].trimmed();
            headerDescription = headerParts[1].trimmed();

            // Вставляем заголовок в базу данных
            query.prepare("INSERT INTO Header_tbl (name, description, dataCount) VALUES (:name, :description, 0) RETURNING id");
            query.bindValue(":name", headerName);
            query.bindValue(":description", headerDescription);

            if (!query.exec() || !query.next()) {
                QMessageBox::critical(this, "Ошибка", "Не удалось добавить заголовок в базу данных:\n" + query.lastError().text());
                file.close();
                return;
            }

            importedHeaders = query.value(0).toInt(); // Получаем ID нового заголовка
        }
        // Если строка не содержит ";", это данные для текущего заголовка
        else {
            if (headerName.isEmpty()) {
                QMessageBox::warning(this, "Ошибка", "Данные указаны без связанного заголовка.");
                file.close();
                return;
            }

            query.prepare("INSERT INTO Data_tbl (data, sv_header) VALUES (:data, :headerId)");
            query.bindValue(":data", line);
            query.bindValue(":headerId", importedHeaders);

            if (!query.exec()) {
                QMessageBox::critical(this, "Ошибка", "Не удалось добавить данные в базу данных:\n" + query.lastError().text());
                file.close();
                return;
            }

            importedData++;
        }
    }

    // Обновляем данные для последнего заголовка
    if (!headerName.isEmpty()) {
        query.prepare("UPDATE Header_tbl SET dataCount = (SELECT COUNT(*) FROM Data_tbl WHERE sv_header = :headerId) WHERE id = :headerId");
        query.bindValue(":headerId", importedHeaders);
        query.exec();
    }

    file.close();
    QMessageBox::information(this, "Успех", QString("Импортировано заголовков: %1\nИмпортировано данных: %2").arg(importedHeaders).arg(importedData));
    loadData(); // Обновляем данные в интерфейсе
}

void MainWindow::exportAllToCSV()
{
    // Выбор файла для сохранения
    QString filePath = QFileDialog::getSaveFileName(this, "Сохранить файл", "all_data.csv", "CSV Files (*.csv)");
    if (filePath.isEmpty()) {
        return;
    }

    // Открываем файл для записи
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Ошибка", "Не удалось открыть файл для записи.");
        return;
    }

    QTextStream out(&file);

    // Запрашиваем все заголовки из Header_tbl
    QSqlQuery headerQuery;
    headerQuery.prepare("SELECT id, name, description FROM Header_tbl");

    if (!headerQuery.exec()) {
        QMessageBox::critical(this, "Ошибка", "Не удалось выполнить запрос к таблице Header_tbl:\n" + headerQuery.lastError().text());
        file.close();
        return;
    }

    // Перебираем заголовки
    while (headerQuery.next()) {
        int headerId = headerQuery.value("id").toInt();
        QString headerName = headerQuery.value("name").toString();
        QString headerDescription = headerQuery.value("description").toString();

        // Записываем заголовок
        out << headerName << ";" << headerDescription << ";\n";

        // Запрашиваем связанные данные из Data_tbl
        QSqlQuery dataQuery;
        dataQuery.prepare("SELECT data FROM Data_tbl WHERE sv_header = :headerId");
        dataQuery.bindValue(":headerId", headerId);

        if (!dataQuery.exec()) {
            QMessageBox::critical(this, "Ошибка", "Не удалось выполнить запрос к таблице Data_tbl:\n" + dataQuery.lastError().text());
            file.close();
            return;
        }

        // Записываем связанные данные
        while (dataQuery.next()) {
            QString dataValue = dataQuery.value("data").toString();
            out << dataValue << "\n";
        }
    }

    file.close();
    QMessageBox::information(this, "Успех", "Все данные успешно экспортированы в файл.");
}


