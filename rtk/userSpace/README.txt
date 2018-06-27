代码结构说明：

1、本套代码是从NH325、NH316代码主线继承过来的，源代码路径为：http://192.168.100.233:18080/svn/ECOS_PLAT_FOR_WIFI/SourceCodes/Branches/NH325_old_platform/BCM5357_1X8/

   更早期的代码路径可以追溯到 N3_ecos和N3 ，如有疑问请回溯查找；


2、本套代码中目前有3套http，tenda_htttp_v3（tenda v3版本）、tenda_http_ucd（从nh325继承过来的ucd风格）和tenda_http（为归一化预留的开发接口），
   
   分别使用宏CONFIG_TENDA_HTTPD_V3、CONFIG_TENDA_HTTPD_UCD、CONFIG_TENDA_HTTPD_NORMAL控制，其中如果需要使用http模块，CONFIG_TENDA_HTTPD宏
   
   必须打开；

3、本套代码中Tenda_normalization.config是最全面的配置集合，开发产品可以通过该配置打开或关闭对应的开关即可；维护代码需要把主线需要的功能的开关

   和到该配置文件中；  


by lvliang@tenda.cn