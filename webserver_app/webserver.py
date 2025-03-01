from flask import Flask, request, render_template, send_from_directory, redirect, url_for
import os
import mimetypes

app = Flask(__name__)

# Thư mục lưu trữ
PICTURES_FOLDER = './uploads/pictures'
VIDEOS_FOLDER = './uploads/videos'

# Tạo thư mục nếu chưa tồn tại
os.makedirs(PICTURES_FOLDER, exist_ok=True)
os.makedirs(VIDEOS_FOLDER, exist_ok=True)

app.config['PICTURES_FOLDER'] = PICTURES_FOLDER
app.config['VIDEOS_FOLDER'] = VIDEOS_FOLDER

@app.route('/')
def index():
    """Trang chính: Hiển thị form upload và danh sách file."""
    pictures = os.listdir(PICTURES_FOLDER)  # Lấy danh sách file ảnh
    videos = os.listdir(VIDEOS_FOLDER)     # Lấy danh sách file video
    message = request.args.get('message', '')  # Nhận thông báo nếu có
    return render_template('index.html', pictures=pictures, videos=videos, message=message)

@app.route('/upload', methods=['POST'])
def upload_file():
    """Xử lý upload file."""
    if 'file' not in request.files:
        return "No file part", 400
    file = request.files['file']
    if file.filename == '':
        return "No selected file", 400

    # Lấy loại file dựa trên phần mở rộng hoặc MIME type
    mime_type, _ = mimetypes.guess_type(file.filename)

    if mime_type and mime_type.startswith('image'):
        filepath = os.path.join(app.config['PICTURES_FOLDER'], file.filename)
        file.save(filepath)
        message = "Image uploaded successfully!"
    elif mime_type and mime_type.startswith('video'):
        filepath = os.path.join(app.config['VIDEOS_FOLDER'], file.filename)
        file.save(filepath)
        message = "Video uploaded successfully!"
    else:
        return "Unsupported file type", 400

    # Redirect về trang chính với thông báo
    return redirect(f"/?message={message}")

@app.route('/download/<folder>/<filename>')
def download_file(folder, filename):
    """Cho phép download file từ thư mục tương ứng."""
    if folder == 'pictures':
        directory = app.config['PICTURES_FOLDER']
    elif folder == 'videos':
        directory = app.config['VIDEOS_FOLDER']
    else:
        return "Invalid folder", 400

    return send_from_directory(directory, filename, as_attachment=True)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
