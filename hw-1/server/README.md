# Первое домашнее задание по курсу Архитектура программных систем

# Подготовка к запуску
## Настройка пользователя и базы в MySQL
sudo mysql
CREATE USER 'stud'@'localhost' IDENTIFIED BY 'stud';
GRANT ALL PRIVILEGES ON * . * TO 'stud'@'localhost';
CREATE DATABASE stud;

Таблицы для базы данных создаются с помощью скриптов в репозитории: 
"fill_articles" - таблица с информацией о статьях; 
"fill_persons" - таблица с личными данными пользователей. 
Данные для этих таблиц частично были сгенерированы с помощью сайта: https://www.onlinedatagenerator.com/

##  Запуск
sudo ./build/hl_mai_lab_01 --host=localhost --port=3306 --login=stud --password=stud --database=stud

# Запросы
index.yaml
