DROP DATABASE IF EXISTS kalonline;

CREATE DATABASE kalonline;
USE kalonline;

CREATE TABLE account (
	idaccount INT AUTO_INCREMENT,
	login VARCHAR(30) NOT NULL,
	password VARCHAR(30) NOT NULL,
	secondary VARCHAR(8),

	PRIMARY KEY (idaccount),
	UNIQUE (login)
);

CREATE TABLE player (
	idplayer INT AUTO_INCREMENT,
	idaccount INT NOT NULL,
	name VARCHAR(30) NOT NULL,
	job TINYINT NOT NULL DEFAULT 1,
	class TINYINT NOT NULL,
	level TINYINT NOT NULL DEFAULT 1,
	map TINYINT NOT NULL DEFAULT 0,
	x INT NOT NULL DEFAULT 258039,
	y INT NOT NULL DEFAULT 259336,
	z INT NOT NULL DEFAULT 16044,
	strength SMALLINT NOT NULL,
	health SMALLINT NOT NULL,
	inteligence SMALLINT NOT NULL,
	wisdom SMALLINT NOT NULL,
	dexterity SMALLINT NOT NULL,
	contribute SMALLINT NOT NULL DEFAULT 0,
	curhp INT NOT NULL,
	curmp SMALLINT NOT NULL,
	exp BIGINT NOT NULL DEFAULT 0,
	pupoint SMALLINT NOT NULL DEFAULT 0,
	supoint SMALLINT NOT NULL DEFAULT 0,
	anger INT NOT NULL DEFAULT 0,
	face TINYINT NOT NULL,
	hair TINYINT NOT NULL,
	deleted TINYINT NOT NULL DEFAULT 0,

	PRIMARY KEY (idplayer),
	FOREIGN KEY (idaccount) 
		REFERENCES account (idaccount),
	UNIQUE(name)
);

CREATE TABLE shortcut (
	idplayer INT AUTO_INCREMENT,
	idslot SMALLINT NOT NULL,
	value SMALLINT NOT NULL DEFAULT 0,

	FOREIGN KEY (idplayer)
		REFERENCES player (idplayer)
);

DROP TRIGGER IF EXISTS player_after_insert;

DELIMITER //
CREATE TRIGGER player_after_insert
AFTER INSERT
	ON player FOR EACH ROW
BEGIN
	INSERT INTO shortcut (idplayer, idslot)
		VALUES
			(NEW.idplayer, 1), (NEW.idplayer, 2), (NEW.idplayer, 3), (NEW.idplayer, 4), (NEW.idplayer, 5),
			(NEW.idplayer, 6), (NEW.idplayer, 7), (NEW.idplayer, 8), (NEW.idplayer, 9), (NEW.idplayer, 10),
			(NEW.idplayer, 11), (NEW.idplayer, 12), (NEW.idplayer, 13), (NEW.idplayer, 14), (NEW.idplayer, 15),
			(NEW.idplayer, 16), (NEW.idplayer, 17), (NEW.idplayer, 18), (NEW.idplayer, 19), (NEW.idplayer, 20);

	IF (NEW.class = 0) THEN #Knight
		INSERT INTO item (idplayer, `index`, curend) VALUES (NEW.idplayer, 1, 4);
	ELSEIF (NEW.class = 1) THEN #Mage
		INSERT INTO item (idplayer, `index`, curend) VALUES (NEW.idplayer, 90, 4);
	ELSEIF (NEW.class = 2) THEN #Archer
		INSERT INTO item (idplayer, `index`, curend) VALUES (NEW.idplayer, 22, 4);
	ELSEIF (NEW.class = 3) THEN #Thief
		INSERT INTO item (idplayer, `index`, curend) VALUES (NEW.idplayer, 1404, 4);
	ELSEIF (NEW.class = 4) THEN #Shaman
		INSERT INTO item (idplayer, `index`, curend) VALUES (NEW.idplayer, 7200, 4);
	END IF;
END; //
DELIMITER ;

CREATE TABLE item (
	iditem INT AUTO_INCREMENT,
	idplayer INT NOT NULL,
	`index` SMALLINT NOT NULL,
	num INT NOT NULL DEFAULT 1,
	info INT NOT NULL DEFAULT 0,
	prefix TINYINT NOT NULL DEFAULT 0,
	curend TINYINT NOT NULL DEFAULT 0,
	xattack TINYINT NOT NULL DEFAULT 0,
	xmagic TINYINT NOT NULL DEFAULT 0,
	xdefense TINYINT NOT NULL DEFAULT 0,
	xhit TINYINT NOT NULL DEFAULT 0,
	xdodge TINYINT NOT NULL DEFAULT 0,
	explosiveblow TINYINT NOT NULL DEFAULT 0,
	fusion TINYINT NOT NULL DEFAULT 0,
	fmeele SMALLINT NOT NULL DEFAULT 0,
	fmagic SMALLINT NOT NULL DEFAULT 0,
	fdefense SMALLINT NOT NULL DEFAULT 0,
	fabsorb SMALLINT NOT NULL DEFAULT 0,
	fevasion TINYINT NOT NULL DEFAULT 0,
	fhit TINYINT NOT NULL DEFAULT 0,
	fhp TINYINT NOT NULL DEFAULT 0,
	fmp TINYINT NOT NULL DEFAULT 0,
	fstr TINYINT NOT NULL DEFAULT 0,
	fhth TINYINT NOT NULL DEFAULT 0,
	fint TINYINT NOT NULL DEFAULT 0,
	fwis TINYINT NOT NULL DEFAULT 0,
	fdex TINYINT NOT NULL DEFAULT 0,
	shot TINYINT NOT NULL DEFAULT 0,
	perforation SMALLINT NOT NULL DEFAULT 0,
	gongleft INT NOT NULL DEFAULT 0,
	gongright INT NOT NULL DEFAULT 0,

	PRIMARY KEY (iditem),
	FOREIGN KEY (idplayer)
		REFERENCES player (idplayer)
);

INSERT INTO account (login, password, secondary) 
	VALUES 
		('bot1', 'passwd', '00000000'),
		('bot2', 'passwd', '00000000'),
		('bot3', 'passwd', '00000000'),
		('bot4', 'passwd', '00000000'),
		('bot5', 'passwd', '00000000');

DELIMITER $$
DROP PROCEDURE IF EXISTS create_bot_accounts $$
CREATE PROCEDURE create_bot_accounts ()
BEGIN
DECLARE i INT DEFAULT 6;
DECLARE accounts_to_create INT DEFAULT 1000;

WHILE i <= accounts_to_create DO
    INSERT INTO account (login, password, secondary) VALUES (CONCAT('bot', i), 'passwd', '00000000');
	SET i = i + 1;
END WHILE;

END $$
DELIMITER ;

CALL create_bot_accounts();
