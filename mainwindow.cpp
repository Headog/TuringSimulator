#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QTimer>
#include <QFileDialog>
#include <ctime>
#include <fstream>

#define WIDE_ALPHABET_B u'\uff22'
#define WIDE_NUMBER_0 u'\uff10'
#define WIDE_NUMBER_1 u'\uff11'
#define WIDE_CHARACTER_CARET u'\uff3e'
#define WIDE_CHARACTER_SPACE u'\u3000'

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    turing = new TuringMachine();
    backup = new TuringMachine();
    QTimer *turing_exec_timer = new QTimer();
    connect(turing_exec_timer, SIGNAL(timeout()), this, SLOT(turing_main_loop()));
    turing_exec_timer->start();
    ui->setupUi(this);
    init();
}

MainWindow::~MainWindow()
{
    delete turing;
    delete backup;
    delete ui;
}

QString memory_to_text(std::vector<enum bit> memory, bool wide)
{
    QString result;
    for (enum bit bit : memory) {
        switch (bit) {
        case BLANK:
            result.push_back(wide ? WIDE_ALPHABET_B : 'B');
            break;
        case ON:
            result.push_back(wide ? WIDE_NUMBER_1 : '1');
            break;
        case OFF:
            result.push_back(wide ? WIDE_NUMBER_0 : '0');
            break;
        default:
            assert("Invalid bit");
        }
    }
    return result;
}

QString pointer_to_text(int pointer)
{
    QString result;
    for (int i = 0; i < pointer; i++) {
        result.push_back(WIDE_CHARACTER_SPACE);
    }
    result.push_back(WIDE_CHARACTER_CARET);
    return result;
}

enum bit text_to_bit(QString bit_text) {
    if (bit_text.length() != 1) {
        throw "未知的比特值：" + bit_text;
    }
    switch (bit_text.toStdWString().c_str()[0]) {
    case 'B':
        return BLANK;
    case '1':
        return ON;
    case '0':
        return OFF;
    default:
        throw "未知的比特值：" + bit_text;
    }
}

QString bit_to_text(enum bit bit) {
    switch (bit) {
    case BLANK:
        return "B";
    case ON:
        return "1";
    case OFF:
        return "0";
    default:
        assert("Invalid bit");
    }
    return nullptr;
}

QString bits_to_text(std::vector<enum bit> bits) {
    QString result;
    for (enum bit bit : bits) {
        result.push_back(bit_to_text(bit));
    }
    return result;
}

std::vector<enum bit> text_to_bits(QString bits_text) {
    std::vector<enum bit> result;
    for (QChar &bit_text : bits_text) {
        result.push_back(text_to_bit(bit_text));
    }
    return result;
}

std::vector<enum bit> random_bits(int num=DEFAULT_BITS) {
    std::vector<enum bit> result;
    for (int i = 0; i < num; i++) {
        result.push_back(std::rand() % 2 == 0 ? OFF : ON);
    }
    return result;
}

std::vector<enum bit> off_bits(int num=DEFAULT_BITS) {
    std::vector<enum bit> result;
    for (int i = 0; i < num; i++) {
        result.push_back(OFF);
    }
    return result;
}

int text_to_offset(QString opt_text) {
    switch (opt_text.toStdWString().c_str()[0]) {
    case 'N':
        return 0;
    case 'L':
        return -1;
    case 'R':
        return 1;
    default:
        throw "未知的操作：" + opt_text;
    }
}

QString offset_to_text(int offset) {
    switch (offset) {
    case 0:
        return "N";
    case -1:
        return "L";
    case 1:
        return "R";
    }
    return "?";
}

struct rule parse_rule(QString rule_text, std::set<std::string> *status) {
    int index = 0;
    struct rule result;
    for (QString &label : rule_text.split(' ')) {
        switch (index) {
        case 0:
            result.if_state = label.toStdString();
            status->insert(label.toStdString());
            break;
        case 1:
            result.if_bit = text_to_bit(label);
            break;
        case 2:
            result.set_bit = text_to_bit(label);
            break;
        case 3:
            result.offset = text_to_offset(label);
            break;
        case 4:
            result.set_state = label.toStdString();
            status->insert(label.toStdString());
            break;
        default:
            throw "参数数量过多";
        }
        index++;
    }
    if (index != 5) {
        throw "参数数量不足";
    }
    return result;
}

QString rule_to_text(struct rule rule, bool opt_only) {
    if (opt_only) {
        return offset_to_text(rule.offset) + bit_to_text(rule.set_bit) + QString::fromStdString(rule.set_state);
    } else {
        return QString::fromStdString(rule.if_state) + ' ' + bit_to_text(rule.if_bit) + ' ' + QString::fromStdString(std::to_string(rule.offset)) + ' ' + bit_to_text(rule.set_bit) + ' ' + QString::fromStdString(rule.set_state);
    }
}

void MainWindow::warn(QString msg) {
    ui->label_warning->setText(msg);
}

void MainWindow::clear_warn() {
    ui->label_warning->setText("");
}

void MainWindow::init()
{
    ui->bar_pointer->setMaximum(POINTER_BAR_LENGTH - 1);
    restart();
}

void MainWindow::update_all() {
    ui->text_memory->setPlainText(memory_to_text(turing->memory, true));
    ui->text_current->setPlainText(memory_to_text(turing->memory, false));
    ui->text_pointer->setPlainText(pointer_to_text(turing->pointer));
    ui->text_pos->setPlainText(QString::fromStdString(std::to_string(turing->pointer)));
    ui->text_status->setPlainText(QString::fromStdString(turing->state));
    ui->bar_pointer->setSliderPosition((int)((double) turing->pointer / (turing->memory.size() - 1) * POINTER_BAR_LENGTH));
    ui->text_step->setPlainText(QString::fromStdString(std::to_string(steps)));
}

void MainWindow::load_rules() {
    turing->rules.clear();
    QString rules_text = ui->text_rules->toPlainText();
    QStringList rule_texts = rules_text.split('\n');
    int i = 0;
    for (QString &rule_text : rule_texts) {
        try {
            turing->rules.push_back(parse_rule(rule_text, &status));
        } catch (const char* error) {
            warn(QString::fromStdString("在第" + std::to_string(i) + "行：" + error));
            return;
        } catch (QString error) {
            warn(QString::fromStdString("在第" + std::to_string(i) + "行：" + error.toStdString()));
            return;
        }
        i++;
    }
}

void MainWindow::step()
{
    struct rule *last_rule_ptr = NULL;
    for (int i = 0; i < execs_at_one_step; i++) {
        last_rule_ptr = turing->step();
        if (last_rule_ptr == NULL) {
            warn("因规则中无可用操作而停机。");
            running = false;
            update_all();
            return;
        }
        steps++;
    }
    if (last_rule_ptr == NULL) {
        warn("内部错误：未执行任何操作。");
        running = false;
        return;
    }
    ui->text_prev_opt->setPlainText(rule_to_text(*last_rule_ptr, true));
    update_all();
}

void MainWindow::restart() {
    running = false;
    steps = 0;
    ui->text_step->setPlainText("0");
    ui->text_prev_opt->setPlainText("");
    turing->reset();
    turing->memory.insert(turing->memory.begin(), primary_bits.begin(), primary_bits.end());
    update_all();
    load_rules();
    clear_warn();
    rule_changed = false;
}

void MainWindow::turing_main_loop()
{
    clock_t now = clock();
    if (!running || next_exec > now)
        return;
    for (int i = 0; i < steps_at_once; i++)
        step();
    next_exec = now + interval;
}

void MainWindow::on_button_start_clicked()
{
    if (running) {
        warn("图灵机已启动。");
        return;
    } else {
        clear_warn();
    }
    if (rule_changed) {
        warn("图灵机规则变更尚未生效。（“重启”以生效）");
    } else {
        clear_warn();
    }
    running = true;
}


void MainWindow::on_button_execute_clicked()
{
    if (rule_changed) {
        warn("图灵机规则变更尚未生效，请先“重启”后再“开始”。");
    } else {
        clear_warn();
    }
    step();
}


void MainWindow::on_button_stop_clicked()
{
    running = false;
}


void MainWindow::on_button_reset_clicked()
{
    steps = 0;
    running = false;
    turing->reset();
    turing->memory.insert(turing->memory.begin(), primary_bits.begin(), primary_bits.end());
    update_all();
}


void MainWindow::on_button_back_clicked()
{
    if (steps > 0)
        steps--;
    ui->text_step->setPlainText(QString::fromStdString(std::to_string(steps)));
}


void MainWindow::on_button_restart_clicked()
{
    restart();
}


void MainWindow::on_bar_pointer_sliderMoved(int position)
{
    if (running) {
        warn("图灵机正在运行时不能手动调节指针位置。");
       return;
    } else {
        clear_warn();
    }
    turing->pointer = (size_t)((double)position / POINTER_BAR_LENGTH * turing->memory.size());
    ui->text_pointer->setPlainText(pointer_to_text(turing->pointer));
}


void MainWindow::on_button_import_clicked()
{
    if (running) {
        warn("图灵机正在运行时不能导入内存。");
        return;
    } else {
        clear_warn();
    }
    primary_bits.clear();
    std::vector<enum bit> import_bits = text_to_bits(ui->text_input->text());
    primary_bits.insert(primary_bits.begin(), import_bits.begin(), import_bits.end());
    restart();
    update_all();
}


void MainWindow::on_button_save_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(nullptr, "选择输出文件所在文件夹");
    std::ofstream fout(dir.toStdString(), std::ios::binary);
    for (enum bit bit: turing->memory) {
        switch (bit) {
        case BLANK:
            warn("保存时忽略空比特位，可能导致乱序。");
            continue;
        case ON:
            fout << 1;
        }
    }
}


void MainWindow::on_button_open_clicked()
{

}


void MainWindow::on_box_speed_valueChanged(double arg1)
{
    interval = (int) (1.0 / arg1 * CLOCKS_PER_SEC);
}


void MainWindow::on_box_step_once_valueChanged(int arg1)
{
    steps_at_once = arg1;
}


void MainWindow::on_box_step_exec_valueChanged(int arg1)
{
    execs_at_one_step = arg1;
}


void MainWindow::on_text_rules_textChanged()
{
    rule_changed = true;
}


void MainWindow::on_button_random_clicked()
{
    ui->text_input->setText(bits_to_text(random_bits()));
}


void MainWindow::on_check_B_clicked()
{
    new_bit = BLANK;
}


void MainWindow::on_check_0_clicked()
{
    new_bit = OFF;
}


void MainWindow::on_check_1_clicked()
{
    new_bit = ON;
}


void MainWindow::on_check_random_clicked()
{
    new_bit = RANDOM;
}

