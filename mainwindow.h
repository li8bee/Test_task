#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlQueryModel>
#include <QProgressBar> // подключаем прогресс-бар

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void loadData();
    void loadDataForSelectedHeader();
    void addHeader();
    void addData();
    void deleteHeader();
    void deleteData();
    void exportHeaderToCSV();
    void importHeaderFromCSV();
    void exportAllToCSV(); // Новый слот для экспорта всех данных

private:
    Ui::MainWindow *ui;
    QSqlQueryModel *headerModel;
    QSqlQueryModel *dataModel;
    QProgressBar *progressBar;  // Добавляем указатель на прогресс-бар
};

#endif // MAINWINDOW_H
