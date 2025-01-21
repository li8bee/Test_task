#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlQueryModel>  // Для работы с произвольными SQL-запросами

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
    void loadData();           // Слот для обработки кнопки загрузки данных
    void loadDataForSelectedHeader();  // Слот для загрузки данных из Data_tbl для выбранного заголовка

private:
    Ui::MainWindow *ui;
    QSqlQueryModel *headerModel; // Модель для отображения данных из Header_tbl
    QSqlQueryModel *dataModel;   // Модель для отображения данных из Data_tbl
};

#endif // MAINWINDOW_H
