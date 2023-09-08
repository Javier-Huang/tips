#include <unistd.h>
#include <libubox/blobmsg_json.h>
#include <libubox/uloop.h>
#include <libubus.h>
 
static struct ubus_context *ctx;
 
static uint32_t obj_id;
static struct ubus_subscriber wpa_notify_event;

enum {
	WPA_EVENT_IFNAME,
	WPA_EVENT_ENCRYPTION,
	WPA_EVENT_SSID,
	WPA_EVENT_BSSID,
	WPA_EVENT_PASSPHRASE,
	WPA_EVENT_CONNECTED,
	WPA_EVENT_REASON,
	WPA_EVENT_MAX
};

static const struct blobmsg_policy wpa_event_policy[WPA_EVENT_MAX] = {
	[WPA_EVENT_IFNAME] = { .name = "ifname", .type = BLOBMSG_TYPE_STRING },
	[WPA_EVENT_ENCRYPTION] = { .name = "encryption", .type = BLOBMSG_TYPE_BOOL },
	[WPA_EVENT_SSID] = { .name = "ssid", .type = BLOBMSG_TYPE_STRING },
	[WPA_EVENT_BSSID] = { .name = "bssid", .type = BLOBMSG_TYPE_STRING },
	[WPA_EVENT_PASSPHRASE] = { .name = "passphrase", .type = BLOBMSG_TYPE_STRING },
	[WPA_EVENT_CONNECTED] = { .name = "connected", .type = BLOBMSG_TYPE_BOOL },
	[WPA_EVENT_REASON] = { .name = "reason", .type = BLOBMSG_TYPE_STRING },
};
 
static void _wpa_sub_timeout_cb(struct uloop_timeout *timeout)
{
	static struct uloop_timeout sub_timer = {
		.cb = _wpa_sub_timeout_cb,
	};

	int ret = ubus_lookup_id(ctx, "wpa_supplicant.wlan0", &obj_id);
	if (ret) {
		fprintf(stderr, "Failed to lookup object: %s\n", ubus_strerror(ret));
		uloop_timeout_set(&sub_timer, 1000);
		return;
	}
 
	/* 订阅object */
	ret = ubus_subscribe(ctx, &wpa_notify_event, obj_id);
	if (ret) {
		fprintf(stderr, "Failed to subscribe: %s\n", ubus_strerror(ret));
		uloop_timeout_set(&sub_timer, 1000);
		return;
	}
}

static void wpa_sub_timeout_cb()
{
	_wpa_sub_timeout_cb(NULL);
}
 
static int wpa_notify_cb(struct ubus_context *ctx, struct ubus_object *obj,
			      struct ubus_request_data *req,
			      const char *method, struct blob_attr *msg)
{
	struct blob_attr *tb[WPA_EVENT_MAX];
	struct blob_attr *cur;

	if (strcmp(method, "wpa_conn_notify") != 0) {
		return 0; 
	}

	blobmsg_parse(wpa_event_policy, WPA_EVENT_MAX, tb, blob_data(msg), blob_len(msg));

	printf("wpa_notify: >>>>>>>>>>>>>>>>>>>>\n");
	printf("method: %s\n", method);
	if ((cur = tb[WPA_EVENT_IFNAME])) {
		printf("ifname: %s\n", blobmsg_get_string(cur));
	}
	if ((cur = tb[WPA_EVENT_SSID])) {
		printf("ssid: %s\n", blobmsg_get_string(cur));
	}
	if ((cur = tb[WPA_EVENT_BSSID])) {
		printf("bssid: %s\n", blobmsg_get_string(cur));
	}
	if ((cur = tb[WPA_EVENT_ENCRYPTION])) {
		printf("encryption: %d\n", blobmsg_get_bool(cur));
	}
	if ((cur = tb[WPA_EVENT_PASSPHRASE])) {
		printf("passphrase: %s\n", blobmsg_get_string(cur));
	}
	if ((cur = tb[WPA_EVENT_CONNECTED])) {
		printf("connected: %d\n", blobmsg_get_bool(cur));
	}
	if ((cur = tb[WPA_EVENT_REASON])) {
		printf("reason: %s\n", blobmsg_get_string(cur));
	}
	printf("wpa_notify: <<<<<<<<<<<<<<<<<<<<\n");
	return 0;
}

static void wpa_notify_remove_cb(struct ubus_context *ctx,
				      struct ubus_subscriber *obj, uint32_t id)
{
	printf("remove handler...\n");
	ubus_unsubscribe(ctx, obj, id); /* 取消订阅 */
	/* ubus_unsubscribe(ctx, &wpa_notify_event, obj_id); /1* 取消订阅 *1/ */
	wpa_sub_timeout_cb();
}
 
static void subscriber_main(void)
{
	int ret;
	
	/* 通知到来时的处理函数。 */
	wpa_notify_event.cb = wpa_notify_cb;
	wpa_notify_event.remove_cb = wpa_notify_remove_cb; //server主动发起删除该client的订阅的cb函数（如server退出的时候）
 
	/* 注册event */
	ret = ubus_register_subscriber(ctx, &wpa_notify_event);
	if (ret)
		fprintf(stderr, "Failed to add watch handler: %s\n", ubus_strerror(ret));
    
	/* 得到要订阅的object的id */
	ret = ubus_lookup_id(ctx, "wpa_supplicant.wlan0", &obj_id);
	if (ret) {
		fprintf(stderr, "Failed to lookup object: %s\n", ubus_strerror(ret));
		wpa_sub_timeout_cb();
		return;
	}
 
	/* 订阅object */
	ret = ubus_subscribe(ctx, &wpa_notify_event, obj_id);
	if (ret) {
		fprintf(stderr, "Failed to subscribe: %s\n", ubus_strerror(ret));
		wpa_sub_timeout_cb();
		return;
	}
}
 
int main(int argc, char **argv)
{
	const char *ubus_socket = NULL;
 
	uloop_init();
 
	ctx = ubus_connect(ubus_socket);
	if (!ctx) {
		fprintf(stderr, "Failed to connect to ubus\n");
		return -1;
	}
 
	ubus_add_uloop(ctx);
 
	subscriber_main();
	
	uloop_run();
 
	ubus_free(ctx);
	uloop_done();
 
	return 0;
}
