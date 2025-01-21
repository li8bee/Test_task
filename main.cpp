#include <QCoreApplication>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // Подключаемся к базе данных PostgreSQL
    QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");
    db.setHostName("localhost");      // Адрес сервера
    db.setDatabaseName("testdb");     // Имя базы данных
    db.setUserName("postgres");       // Имя пользователя
    db.setPassword("admin");       // Пароль

    if (!db.open()) {
        qDebug() << "Ошибка подключения:" << db.lastError().text();
        return -1;
    }

    qDebug() << "Соединение с базой данных успешно!";

    // Выполняем запрос для получения данных из таблицы Header_tbl
    QSqlQuery query;
    if (!query.exec("SELECT * FROM data_tbl")) {
        qDebug() << "Ошибка выполнения запроса:" << query.lastError().text();
        return -1;
    }

    // Выводим данные из таблицы в консоль
    while (query.next()) {
        int id = query.value("id").toInt();
        QString name = query.value("name").toString();
        QString description = query.value("description").toString();
        int dataCount = query.value("datacount").toInt();

        qDebug() << "ID:" << id
                 << ", Name:" << name
                 << ", Description:" << description
                 << ", Data Count:" << dataCount;
    }

    return 0;
}
