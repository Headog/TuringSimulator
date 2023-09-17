#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <turingmachine.h>
#include <ctime>
#include <set>

constexpr int POINTER_BAR_LENGTH = 256;
constexpr int DEFAULT_BITS = 8;

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
    void on_button_start_clicked();

    void on_button_execute_clicked();

    void on_button_stop_clicked();

    void on_button_reset_clicked();

    void on_button_back_clicked();

    void on_bar_pointer_sliderMoved(int position);

    void on_button_import_clicked();

    void on_button_save_clicked();

    void on_button_open_clicked();

    void on_box_speed_valueChanged(double arg1);

    void on_box_step_once_valueChanged(int arg1);

    void on_box_step_exec_valueChanged(int arg1);

    void turing_main_loop();

    void on_button_restart_clicked();

    void on_text_rules_textChanged();

    void on_button_random_clicked();

    void on_check_B_clicked();

    void on_check_0_clicked();

    void on_check_1_clicked();

    void on_check_random_clicked();

private:
    Ui::MainWindow *ui;
    TuringMachine *turing;
    TuringMachine *backup;
    bool running = false;
    bool rule_changed = false;
    clock_t next_exec = 0;
    int interval = CLOCKS_PER_SEC; // milliseconds
    int steps = 0;
    int steps_at_once = 1;
    int execs_at_one_step = 1;
    std::set<std::string> status; // Deprecated
    std::vector<enum bit> primary_bits = {OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF};

    void init();

    void update_all();

    void load_rules();

    void step();

    void restart();

    void warn(QString msg);

    void clear_warn();

    int parse_pointer_position(int position_of_bar);

    std::vector<enum bit> parse_memory();

};
#endif // MAINWINDOW_H
