#!/usr/bin/env bash

# this test requires running skm server and configured ~/.odbc.ini
# cp -n /usr/share/doc/skm-odbc/examples/odbc.ini ~/.odbc.ini
# apt install unixodbc

# to build and install package:
# cd .. && debuild -us -uc -i --source-option=--format="3.0 (native)" && sudo dpkg -i `ls ../skm-odbc_*_amd64.deb | tail -n1`

# test https:
# cd .. && debuild -eDH_VERBOSE=1 -eCMAKE_FLAGS="-DENABLE_SSL=1 -DFORCE_STATIC_LINK=" -us -uc -i --source-option=--format="3.0 (native)" && sudo dpkg -i `ls ../skm-odbc_*_amd64.deb | tail -n1`

function q {
    echo "Asking [$*]"
    echo "$*" | isql skm -b -v
}

q "SELECT * FROM system.build_options;"
q "CREATE DATABASE IF NOT EXISTS test;"
q "DROP TABLE IF EXISTS test.odbc1;"
q "CREATE TABLE test.odbc1 (ui64 UInt64, string String, date Date, datetime DateTime) ENGINE = Memory;"
q "INSERT INTO test.odbc1 VALUES (1, '2', 3, 4);"
q "SELECT * FROM test.odbc1 WHERE ui64=1;"

q 'SELECT {fn CONVERT(1, SQL_BIGINT)}'
q "SELECT {fn CONVERT(100000, SQL_TINYINT)}"
q "SELECT {fn CONCAT('a', 'b')}"
q 'SELECT CAST({fn TRUNCATE(1.1 + 2.4, 1)} AS INTEGER) AS `yr_date_ok`'

q $'SELECT COUNT({fn ABS(`test`.`odbc1`.`ui64`)}) FROM test.odbc1'

q $'SELECT {fn TIMESTAMPDIFF(SQL_TSI_DAY,CAST(`test`.`odbc1`.`datetime` AS DATE),CAST(`test`.`odbc1`.`date` AS DATE))} AS `Calculation_503558746242125826`, SUM({fn CONVERT(1, SQL_BIGINT)}) AS `sum_Number_of_Records_ok` FROM `test`.`odbc1` WHERE (CAST(`test`.`odbc1`.`datetime` AS DATE) <> {d \'1970-01-01\'}) GROUP BY `Calculation_503558746242125826`'

q $'SELECT COUNT({fn ABS(`test`.`odbc1`.`ui64`)}) AS `TEMP_Calculation_559572257702191122__2716881070__0_`, SUM({fn ABS(`test`.`odbc1`.`ui64`)}) AS `TEMP_Calculation_559572257702191122__3054398615__0_`  FROM test.odbc1;'

q $'SELECT SUM((CASE WHEN (`test`.`odbc1`.`ui64` * `test`.`odbc1`.`ui64`) < 0 THEN NULL ELSE {fn SQRT((`test`.`odbc1`.`ui64` * `test`.`odbc1`.`ui64`))} END)) AS `TEMP_Calculation_559572257701634065__1464080195__0_`, COUNT((CASE WHEN (`test`.`odbc1`.`ui64` * `test`.`odbc1`.`ui64`) < 0 THEN NULL ELSE {fn SQRT((`test`.`odbc1`.`ui64` * `test`.`odbc1`.`ui64`))} END)) AS `TEMP_Calculation_559572257701634065__2225718044__0_` FROM test.odbc1;'

#SELECT (CASE WHEN (NOT = 'True') OR (`test`.`odbc1`.`string` = 'True') OR (`test`.`odbc1`.`string2` = 'True') THEN 1 WHEN NOT (NOT = 'True') OR (`test`.`odbc1`.`string` = 'True') OR (`test`.`odbc1`.`string` = 'True') OR (`test`.`odbc1`.`string2` = 'True') THEN 0 ELSE NULL END) AS `Calculation_597289912116125696`,
#SUM({fn CONVERT(1, SQL_BIGINT)}) AS `sum_Number_of_Records_ok` FROM `test`.`odbc1` GROUP BY `Calculation_597289912116125696`, `string`, `ui64`

q "DROP TABLE IF EXISTS test.purchase_stat;"
q "CREATE TABLE test.purchase_stat (purchase_id UInt64, purchase_date DateTime, offer_category UInt64, amount UInt64) ENGINE = Memory;"
q $'SELECT SUM({fn CONVERT(Custom_SQL_Query.amount, SQL_BIGINT)}) AS sum_amount FROM (SELECT purchase_date, offer_category, SUM(amount) AS amount, COUNT(DISTINCT purchase_id) AS purchase_id FROM test.purchase_stat WHERE (offer_category = 1) GROUP BY purchase_date, offer_category) Custom_SQL_Query HAVING (COUNT(1) > 0)'
q $'SELECT (CASE WHEN (`test`.`odbc1`.`ui64` > 0) THEN 1 WHEN NOT (`test`.`odbc1`.`ui64` > 0) THEN 0 ELSE NULL END) AS `Calculation_162692564973015040`, SUM({fn CONVERT(1, SQL_BIGINT)}) AS `sum_Number_of_Records_ok` FROM `test`.`odbc1` GROUP BY (CASE WHEN (`test`.`odbc1`.`ui64` > 0) THEN 1 WHEN NOT (`test`.`odbc1`.`ui64` > 0) THEN 0 ELSE NULL END)'
q $"SELECT {d '2017-08-30'}"
q 'SELECT CAST(CAST(`odbc1`.`date` AS DATE) AS DATE) AS `tdy_Calculation_687361904651595777_ok` FROM `test`.`odbc1`'

q 'SELECT {fn CURDATE()}'

q $'SELECT `test`.`odbc1`.`ui64` AS `bannerid`, SUM((CASE WHEN `test`.`odbc1`.`ui64` = 0 THEN NULL ELSE `test`.`odbc1`.`ui64` / `test`.`odbc1`.`ui64` END)) AS `sum_Calculation_582934706662502402_ok`, SUM(`test`.`odbc1`.`ui64`) AS `sum_clicks_ok`, SUM(`test`.`odbc1`.`ui64`) AS `sum_shows_ok`, SUM(`test`.`odbc1`.`ui64`) AS `sum_true_installs_ok`, CAST(CAST(`test`.`odbc1`.`date` AS DATE) AS DATE) AS `tdy_Calculation_582934706642255872_ok` FROM `test`.`odbc1` WHERE (`test`.`odbc1`.`string` = \'YandexBrowser\') GROUP BY `test`.`odbc1`.`ui64`, CAST(CAST(`test`.`odbc1`.`date` AS DATE) AS DATE)'

#TODO: q $'SELECT SUM({fn CONVERT(1, SQL_BIGINT)}) AS `sum_Number_of_Records_ok`, CAST({fn TRUNCATE(EXTRACT(YEAR FROM `m_ru_6p_v1`.`date`),0)} AS INTEGER) AS `yr_date_ok` FROM `m_ru_6p_v1` GROUP BY CAST({fn TRUNCATE(EXTRACT(YEAR FROM `m_ru_6p_v1`.`date`),0)} AS INTEGER)'

q $'SELECT test.odbc1.ui64 AS BannerID,   SUM((CASE WHEN test.odbc1.ui64 = 0 THEN NULL ELSE test.odbc1.ui64 / test.odbc1.ui64 END)) AS sum_Calculation_500744014152380416_ok,   SUM(test.odbc1.ui64) AS sum_ch_installs_ok,   SUM(test.odbc1.ui64) AS sum_goodshows_ok FROM test.odbc1 GROUP BY test.odbc1.ui64'
q $'SELECT test.odbc1.ui64 AS BannerID,   SUM((CASE WHEN test.odbc1.ui64 > 0 THEN NULL ELSE test.odbc1.ui64 / test.odbc1.ui64 END)) AS sum_Calculation_500744014152380416_ok,   SUM(test.odbc1.ui64) AS sum_ch_installs_ok,   SUM(test.odbc1.ui64) AS sum_goodshows_ok FROM test.odbc1 GROUP BY test.odbc1.ui64'


q "DROP TABLE IF EXISTS test.test_tableau;"
q "create table test.test_tableau (country String, clicks UInt64, shows UInt64) engine Log"
q "insert into test.test_tableau values ('ru',10000,100500),('ua',1000,6000),('by',2000,6500),('tr',100,500)"
q "insert into test.test_tableau values ('undefined',0,2)"
q "insert into test.test_tableau values ('injected',1,0)"
q 'SELECT test.test_tableau.country AS country, SUM((CASE WHEN test.test_tableau.shows = 0 THEN NULL ELSE CAST(test.test_tableau.clicks AS FLOAT) / test.test_tableau.shows END)) AS sum_Calculation_920986154656493569_ok, SUM({fn POWER(CAST(test.test_tableau.clicks AS FLOAT),2)}) AS sum_Calculation_920986154656579587_ok FROM test.test_tableau GROUP BY test.test_tableau.country;'
q "DROP TABLE test.test_tableau;"

q 'SELECT NULL'
q 'SELECT [NULL]'

q "DROP TABLE IF EXISTS test.adv_watch;"
q "create table test.adv_watch (rocket_date Date, rocket_datetime dateTime, ivi_id UInt64) engine Log"
q "insert into test.adv_watch values (1,2,3)"
q "insert into test.adv_watch values (1, {fn TIMESTAMPADD(SQL_TSI_DAY,-8,CAST({fn CURRENT_TIMESTAMP(0)} AS DATE))}, 3)"
q 'SELECT `test`.`adv_watch`.`rocket_date` AS `rocket_date`, COUNT(DISTINCT `test`.`adv_watch`.`ivi_id`) AS `usr_Calculation_683139814283419648_ok` FROM `test`.`adv_watch` WHERE ((`adv_watch`.`rocket_datetime` >= {fn TIMESTAMPADD(SQL_TSI_DAY,-9,CAST({fn CURRENT_TIMESTAMP(0)} AS DATE))}) AND (`test`.`adv_watch`.`rocket_datetime` < {fn TIMESTAMPADD(SQL_TSI_DAY,1,CAST({fn CURRENT_TIMESTAMP(0)} AS DATE))})) GROUP BY `test`.`adv_watch`.`rocket_date`'
#TODO: q 'SELECT CAST({fn TRUNCATE(EXTRACT(YEAR FROM `adv_watch`.`rocket_date`),0)} AS INTEGER) AS `yr_rocket_date_ok` FROM `adv_watch` GROUP BY CAST({fn TRUNCATE(EXTRACT(YEAR FROM `adv_watch`.`rocket_date`),0)} AS INTEGER)'
q "DROP TABLE test.adv_watch;"

# https://github.com/yandex/skm-odbc/issues/43
q 'DROP TABLE IF EXISTS test.gamoraparams;'
q 'CREATE TABLE test.gamoraparams ( user_id Int64, date Date, dt DateTime, p1 Nullable(Int32), platforms Nullable(Int32), max_position Nullable(Int32), vv Nullable(Int32), city Nullable(String), third_party Nullable(Int8), mobile_tablet Nullable(Int8), mobile_phone Nullable(Int8), desktop Nullable(Int8), web_mobile Nullable(Int8), tv_attach Nullable(Int8), smart_tv Nullable(Int8), subsite_id Nullable(Int32), view_in_second Nullable(Int32), view_in_second_presto Nullable(Int32)) ENGINE = MergeTree(date, user_id, 8192)'
q 'insert into test.gamoraparams values (1, {fn CURRENT_TIMESTAMP }, CAST({fn CURRENT_TIMESTAMP(0)} AS DATE), Null, Null,Null,Null,Null, Null,Null,Null,Null,Null,Null,Null,Null,Null,Null);'
q 'SELECT `Custom_SQL_Query`.`platforms` AS `platforms` FROM (select platforms from test.gamoraparams where platforms is null limit 1) `Custom_SQL_Query` GROUP BY `platforms`'
q 'DROP TABLE test.gamoraparams;'

exit


q 'DROP TABLE IF EXISTS test.increment;'
q 'CREATE TABLE test.increment (n UInt64) engine Log;'

NUM=${NUM=100}
for i in `seq 1 ${NUM}`; do
    q "insert into test.increment values ($i);" > /dev/null
    q 'select * from test.increment;' > /dev/null
done

echo "should be ${NUM}:"
q 'select count(*) from test.increment;'

q 'DROP TABLE test.increment;'


echo "\n\n\nLast log:\n"
cat /tmp/skm-odbc-stderr.$USER
