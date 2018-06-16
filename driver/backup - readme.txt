V1.0 
函数与括号对处理完成，还需要添加：
1）时间处理；
2）convert处理

V1.1 
上一版本遗留问题解决
解决问题：
1）加减乘除的运算符处理添加
2）decode对应到函数transform 增加null处理
待解决：
还未对tableau进行全方位测试

v1.2
1）TIMESTAMPADD处理添加
2）SPLIT处理添加
3）EXTRACT处理增加
4）DAYOFYEAR处理


核心查询语句问题：
3 字段与表名严格区分大小写？
4 to_date函数处理:to_date(DATATIME,'yyyymm')
LTRIM函数待添加