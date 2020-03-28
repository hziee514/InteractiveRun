# Interactive Run
当程序以系统服务方式部署时，让它创建的子进程运行在桌面环境下

## 配置

文件名：在exe文件目录下同名exe文件名加上.ini后缀

```ini
[Window]
# 参考: STARTUPINFO.wShowWindow
# 默认值: 1
show=1
# 参考: STARTUPINFO.dwFlags
# 默认值: 1
flags=1
```



## 演示

参考winsw

```bash
# 安装服务
WinSW.NET4.exe install
# 启动服务，这时会在桌面上打开子程序
WinSW.NET4.exe start
# 停止服务
WinSW.NET4.exe stop
# 卸载服务
WinSW.NET4.exe uninstall
```

