# Training Data AI4Math

Vì các file data không có dung lượng lớn, **database của nhóm mình được lưu trên GitHub** tại đường link:

[https://github.com/danhkhai07/training-data-AI4Math](https://github.com/danhkhai07/training-data-AI4Math)

Nếu ai chưa có quyền truy cập, vui lòng nhắn cho mình (**Nguyễn Danh Khải trên Zalo**) **email GitHub** qua nhóm Zalo chung của topic.  

---

## Note về các executable hỗ trợ

Có **2 chương trình** để hỗ trợ mọi người trong việc tạo file và xóa file lỗi:

- **adder.exe**: tự động prompt tên, ID, lưu cache, và auto pull/push  
- **remover.exe**: prompt user ID và tên file, tự động đổi tên các file, và auto pull/push  

**Lưu ý:**

- Cả 2 chương trình trên yêu cầu mọi người **đã clone repo về**.  
- Chỉ sử dụng khi đã clone repo và cấu hình GitHub của mình.  
- Chương trình được viết bằng AI, có thể lỗi, nên nếu có vấn đề gì vui lòng nhắn trực tiếp với mình qua Zalo.  

---

## Quy chuẩn cấu trúc dữ liệu

1. **Tạo folder riêng cho mỗi người** với **ID không trùng lặp** và tên của mình để tránh lẫn lộn dữ liệu.  
2. **Định dạng file**: `.tex` hoặc `.lean`  
   - Nội dung: văn bản đề bài và đáp án (nếu có) dưới dạng **LaTeX**  
   - Tiêu đề file theo cấu trúc:

[WS/NS][ID người tạo][STT data][Chương][Độ khó]_[MATH/LEAN].*

---
## Mã ID

- **WS**: có đáp án (**With Solution**)  
- **NS**: không có đáp án (**No Solution**)  

## Mã chương

| Mã   | Chương |
|------|--------|
| C01  | Phương trình tuyến tính trong Đại số tuyến tính |
| C02  | Đại số ma trận |
| C03  | Định thức |
| C04  | Không gian vectơ |
| C05  | Giá trị riêng và vectơ riêng |
| C06  | Trực giao và bình phương tối thiểu |
| C07  | Ma trận đối xứng và dạng toàn phương |
| C08  | Hình học của không gian vectơ |
| C09  | Tối ưu hóa |
| C10  | Chuỗi Markov trạng thái hữu hạn |

## Mã độ khó

- **L1**: Dễ nhất → **L5**: Khó nhất  
> Lưu ý: đánh giá độ khó theo cảm nhận cá nhân.

## Phân loại toán

- **MATH**: file `.tex` chứa nội dung LaTeX  
- **LEAN**: file `.lean` chứa logic của Lean
