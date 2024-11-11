#include <iostream>
#include <mysql/mysql.h>

class MySQLDatabase {
public:
    MySQLDatabase(const char* host, const char* user, const char* password, const char* dbname) {
        conn = mysql_init(NULL);
        if (!conn) {
            std::cerr << "mysql_init() failed\n";
            exit(EXIT_FAILURE);
        }

        conn = mysql_real_connect(conn, host, user, password, dbname, 0, NULL, 0);
        if (!conn) {
            std::cerr << "mysql_real_connect() failed: " << mysql_error(conn) << "\n";
            exit(EXIT_FAILURE);
        }
    }

    ~MySQLDatabase() {
        if (conn) {
            mysql_close(conn);
        }
    }

    void createTable(const char* query) {
        if (mysql_query(conn, query)) {
            std::cerr << "CREATE TABLE failed: " << mysql_error(conn) << "\n";
        } else {
            std::cout << "Table created successfully\n";
        }
    }

    void deleteTable(const char* query) {
        if (mysql_query(conn, query)) {
            std::cerr << "DROP TABLE failed: " << mysql_error(conn) << "\n";
        } else {
            std::cout << "Table deleted successfully\n";
        }
    }

    void insertData(const char* query) {
        if (mysql_query(conn, query)) {
            std::cerr << "INSERT failed: " << mysql_error(conn) << "\n";
        } else {
            std::cout << "Data inserted successfully\n";
        }
    }

    void deleteData(const char* query) {
        if (mysql_query(conn, query)) {
            std::cerr << "DELETE failed: " << mysql_error(conn) << "\n";
        } else {
            std::cout << "Data deleted successfully\n";
        }
    }

    void updateData(const char* query) {
        if (mysql_query(conn, query)) {
            std::cerr << "UPDATE failed: " << mysql_error(conn) << "\n";
        } else {
            std::cout << "Data updated successfully\n";
        }
    }

    void queryData(const char* query) {
        if (mysql_query(conn, query)) {
            std::cerr << "SELECT failed: " << mysql_error(conn) << "\n";
            return;
        }

        MYSQL_RES* res = mysql_store_result(conn);
        if (!res) {
            std::cerr << "mysql_store_result() failed: " << mysql_error(conn) << "\n";
            return;
        }

        int num_fields = mysql_num_fields(res);
        MYSQL_ROW row;

        while ((row = mysql_fetch_row(res))) {
            for (int i = 0; i < num_fields; i++) {
                std::cout << (row[i] ? row[i] : "NULL") << " ";
            }
            std::cout << "\n";
        }

        mysql_free_result(res);
    }

private:
    MYSQL* conn;
};

int main() {
    MySQLDatabase db("localhost", "root", "root", "database_cpp");

    db.createTable("CREATE TABLE test (id INT PRIMARY KEY, name VARCHAR(50))");
    db.insertData("INSERT INTO test (id, name) VALUES (1, 'John Doe')");
    db.queryData("SELECT * FROM test");
    db.updateData("UPDATE test SET name = 'Jane Doe' WHERE id = 1");
    db.queryData("SELECT * FROM test");
    db.deleteData("DELETE FROM test WHERE id = 1");
    db.deleteTable("DROP TABLE test");

    return 0;
}