#import "ViewController.h"
#import <sys/socket.h>
#import <netinet/in.h>
#import <arpa/inet.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wobjc-string-compare"

@interface ViewController (){
    NSInteger _protocolIndex; //0:TCP,1:UDP
    int _udp_clientSockfd;//客户端端套接字描述符
    NSString *_loc_ipAdr,*_loc_port ,*_dest_ipAdr,*_dest_port;
    int success;

}
@property (weak, nonatomic) IBOutlet UITextField *des_ipAdress;
@property (weak, nonatomic) IBOutlet UITextField *des_port;
@property (weak, nonatomic) IBOutlet UITextField *sendTF;
@property (weak, nonatomic) IBOutlet UITextView *recvTextView;
@property (weak, nonatomic) IBOutlet UISegmentedControl *segmentControl;

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    _loc_ipAdr = @"127.0.0.1";
    _loc_port = @"10001";
    _dest_ipAdr = _dest_port = @"";
    [self chose:self.segmentControl];
    [NSThread detachNewThreadSelector:@selector(creatUDPSocket) toTarget:self withObject:nil];

}

- (void)creatUDPSocket {

    _udp_clientSockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (_udp_clientSockfd > 0) {
        NSLog(@"UDP socket创建成功");
        success = 1;

    } else {
        NSLog(@"UDP socket创建失败");
        success = 0;
    }
}


- (IBAction)send:(id)sender{
    
    if (!self.sendTF.text.length) {
        return;
    }
    if (!success) {
        NSString *str = [NSString stringWithFormat: @"socket创建失败，无法传输消息！"];
        dispatch_async(dispatch_get_main_queue(), ^{
            self->_recvTextView.text = str;
        });
    }
    
    ssize_t sendLen = 0;

    struct sockaddr_in des_addr;
    bzero(&des_addr, sizeof(des_addr));
    // 发送数据
    ssize_t recvLen = 0;
    char buf[256];
    do {
        _dest_ipAdr = _des_ipAdress.text;
        _dest_port = _des_port.text;
        des_addr.sin_family      = AF_INET;
        des_addr.sin_port        = htons(_dest_port.intValue);
        des_addr.sin_addr.s_addr = inet_addr(_dest_ipAdr.UTF8String);
        sendLen = sendto(_udp_clientSockfd, _sendTF.text.UTF8String, strlen(_sendTF.text.UTF8String), 0,
                (struct sockaddr *)&des_addr, sizeof(des_addr));
        if (sendLen < 0) {
            NSLog(@"传输失败");
            continue;
        }


        socklen_t des_addr_len = sizeof(des_addr);
        recvLen = recvfrom(_udp_clientSockfd, buf, sizeof(buf), 0, (struct sockaddr*)&des_addr, &des_addr_len);
        if (recvLen > 0) {
            NSString *recvStr = [NSString stringWithFormat:@"[UDP消息][来自服务端%@:%@]：%@\n", _dest_ipAdr, _dest_port, [NSString stringWithUTF8String:buf]];
            dispatch_async(dispatch_get_main_queue(), ^{
                self->_recvTextView.text = recvStr;
            });
        }
        else {
            NSString *recvStr = [NSString stringWithFormat:@"消息接收失败，正在尝试重传。"];
            dispatch_async(dispatch_get_main_queue(), ^{
                self->_recvTextView.text = recvStr;
            });
        }
    }while (recvLen <= 0);

}

- (IBAction)chose:(UISegmentedControl *)sender {

    _protocolIndex = sender.selectedSegmentIndex;
    if (sender.selectedSegmentIndex == 0) {
        NSLog(@"选择UDP");
    }
    if (sender.selectedSegmentIndex == 1) {
        NSLog(@"选择TCP");

    }
}

@end

#pragma clang diagnostic pop
