CREATE TABLE amsdb.customers (
    id INT AUTO_INCREMENT PRIMARY KEY,
    first_name VARCHAR(50) NOT NULL,
    last_name VARCHAR(50) NOT NULL
);

INSERT INTO amsdb.customers (first_name, last_name) VALUES
('Liam', 'Smith'),
('Olivia', 'Johnson'),
('Noah', 'Williams'),
('Emma', 'Brown'),
('Oliver', 'Jones'),
('Ava', 'Garcia'),
('Elijah', 'Miller'),
('Sophia', 'Davis'),
('James', 'Rodriguez'),
('Isabella', 'Martinez');


CREATE TABLE amsdb.special_table (
    id INT AUTO_INCREMENT PRIMARY KEY,
    first_name VARCHAR(50) NOT NULL,
    last_name VARCHAR(50) NOT NULL
);

INSERT INTO amsdb.special_table (first_name, last_name) VALUES
('Olivia', 'Johnson'),
('Noah', 'Williams'),
('Oliver', 'Jones'),
('Isabella', 'Martinez');
