#import "ViewController.h"
#import <sys/socket.h>
#import <netinet/in.h>
#import <arpa/inet.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat"
@interface ViewController ()
{
    NSInteger _protocolIndex;//0:TCP,1:UDP
    NSString *_loc_ipAdr,*_loc_port; //*_des_ipAdress, *_des_port;
    int _udp_serverSockfd;//服务端套接字描述符
    
}
@end

@implementation ViewController
- (void)viewDidLoad {
    [super viewDidLoad];
    _loc_ipAdr = @"127.0.0.1"; // 本地ip
    _loc_port = @"10000"; // 本地端口

    [NSThread detachNewThreadSelector:@selector(creatUDPSocket) toTarget:self withObject:nil];
}

- (void)creatUDPSocket{
    _udp_serverSockfd  = socket(AF_INET, SOCK_DGRAM, 0);// ipv4, 数据报式
    if (_udp_serverSockfd > 0 ) {
        NSLog(@"UDP socket创建成功");
        [self UDPBind];
    }else {
        NSLog(@"UDP socket创建失败");
        
    }
}

- (void)UDPBind {
    //获取本地地址
    struct sockaddr_in loc_addr;
    //清空指向的内存中的存储内容，因为分配的内存是随机的
    memset(&loc_addr, 0, sizeof(loc_addr));
    //    loc_addr.sin_len = sizeof(struct sockaddr_in);
    //设置协议族
    loc_addr.sin_family = AF_INET;
    //设置端口
    loc_addr.sin_port = htons(_loc_port.intValue);
    //设置IP地址
    loc_addr.sin_addr.s_addr = inet_addr(_loc_ipAdr.UTF8String);
    //绑定
    int udpCode = bind(_udp_serverSockfd, (const struct sockaddr *)&loc_addr,sizeof(loc_addr) );
    
    if (udpCode == 0) {
        NSLog(@"UDP socket绑定成功");
        [self UDPRecv];
    }else{
        NSLog(@"UDP socekt绑定失败");
        
    }
}


#pragma mark - UDP接收
- (void)UDPRecv{
    // 目标地址
    struct sockaddr_in des_addr;
    // 清空指向的内存中的存储内容   如果不清空会？
    bzero(&des_addr, sizeof(des_addr));

    char buf[1024];
    //清空指向的内存中的存储内容
    while(1) {
        // 接收数据
        bzero(buf, sizeof(buf));
        socklen_t des_addr_len = sizeof(des_addr); // 字节长度
        ssize_t recvLen = recvfrom(_udp_serverSockfd, buf, sizeof(buf), 0, (struct sockaddr *) &des_addr, &des_addr_len);
        // 通过函数recvfrom将message放入到buf中  buf存储获取的长度
        NSString *recvStr, *sendMsg;
        NSString * time = [self getTime:0];
        NSString * date = [self getTime:1];
        if (recvLen > 0) {
            if (strcasecmp(buf, "date") == 0) {
                sendMsg = date;
            } else if (strcasecmp(buf, "time") == 0) {
                sendMsg = time;
            } else {
                sendMsg = @"错误信息";
            }


            NSLog([NSString stringWithUTF8String:inet_ntoa(des_addr.sin_addr)]);
            recvStr = [NSString stringWithFormat:
                    @"%@   %@   [UDP消息][来自客户端%@ : %hu]：请求【%@】，响应【%@】\n", date, time,
                    [NSString stringWithUTF8String:inet_ntoa(des_addr.sin_addr)], des_addr.sin_port, [NSString stringWithUTF8String:buf], sendMsg];
            sendto(_udp_serverSockfd, sendMsg.UTF8String, strlen(sendMsg.UTF8String), 0,
                    (struct sockaddr *)&des_addr, sizeof(des_addr));
            dispatch_async(dispatch_get_main_queue(), ^{
                self->_recTextView.string = [_recTextView.string stringByAppendingString:recvStr];

            });
        }
    }
}


#pragma mark - 选择协议
- (IBAction)choseProtocol:(NSComboBox *)sender {
    if ([sender.stringValue isEqualToString:@"TCP"]) {
        NSLog(@"选择TCP");
        _protocolIndex = 0;
    }
    if ([sender.stringValue isEqualToString:@"UDP"]) {
        NSLog(@"选择UDP");
        _protocolIndex = 1;
    }
}

- (void)setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];

    // Update the view, if already loaded.
}

#pragma mark - 获取当前时间日期

- (NSString *)getTime:(int)choose {
    NSDateFormatter *dateFormatter=[[NSDateFormatter alloc] init];
    if (choose == 1) {
        [dateFormatter setDateFormat:@"YYYY-MM-dd"];
        NSString *date = [dateFormatter stringFromDate:[NSDate date]];
        return date;
    }
    else {
        [dateFormatter setDateFormat:@"hh:mm:ss"];
        NSString *time = [dateFormatter stringFromDate:[NSDate date]];
        return time;
    }
}

@end
