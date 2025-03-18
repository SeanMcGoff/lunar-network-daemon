#include <functional>
#include <libnetfilter_queue/libnetfilter_queue.h>
#include <memory>

class NetfilterQueue {
public:
  NetfilterQueue()
      : handle_(nullptr, nfq_close), queue_handle_(nullptr, nullptr) {}

  void run();

private:
  // this is a "static bridge" pattern which is required for interfacing C++
  // logic with C libraries that use callbacks
  // The static callback has the exact signature the C libary expects, it
  // receives the "this" pointer through the data parameter,
  // Acting as a bridge to the actual instance method
  static int packetCallbackStatic(struct nfq_q_handle *qh,
                                  struct nfgenmsg *nfmsg, struct nfq_data *nfa,
                                  void *data);

  // The actual callback method has access to all the object's members and state
  int packetCallback(struct nfq_q_handle *qh, struct nfgenmsg *nfmsg,
                     struct nfq_data *nfa);

  int fd_;

  // smart pointers for resource management
  std::unique_ptr<struct nfq_handle, decltype(&nfq_close)> handle_;
  std::unique_ptr<struct nfq_q_handle,
                  std::function<void(struct nfq_q_handle *)>>
      queue_handle_;
};