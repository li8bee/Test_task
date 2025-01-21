#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlQueryModel>

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
    void exportHeaderToCSV(); // Новый слот для экспорта

private:
    Ui::MainWindow *ui;
    QSqlQueryModel *headerModel;
    QSqlQueryModel *dataModel;
};

#endif // MAINWINDOW_H
