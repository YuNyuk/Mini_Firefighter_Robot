package com.example.fireapp;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.AsyncTask;
import android.os.Bundle;
import android.util.Log;
import android.view.ViewTreeObserver;
import android.widget.ImageView;
import androidx.appcompat.app.AppCompatActivity;

import java.io.IOException;
import java.io.InputStream;
import java.net.Socket;

public class MainActivity extends AppCompatActivity {

    private ImageView imageView1;
    private ImageView imageView2;
    private String serverIP = "서버 IP";  // 서버 IP
    private int serverPort = 5002;  // 서버 포트

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        imageView1 = findViewById(R.id.imageView1);
        imageView2 = findViewById(R.id.imageView2);

        // ViewTreeObserver를 사용하여 ImageView의 크기 얻기
        imageView1.getViewTreeObserver().addOnGlobalLayoutListener(new ViewTreeObserver.OnGlobalLayoutListener() {
            @Override
            public void onGlobalLayout() {
                imageView1.getViewTreeObserver().removeOnGlobalLayoutListener(this);
                new VideoStreamClient().execute(); // 비디오 스트림 클라이언트 시작
            }
        });
    }

    private class VideoStreamClient extends AsyncTask<Void, Object[], Void> {

        @Override
        protected Void doInBackground(Void... voids) {
            try {
                Socket socket = new Socket(serverIP, serverPort);
                InputStream inputStream = socket.getInputStream();

                while (true) {
                    byte[] sizeBuffer = new byte[16];
                    int sizeRead = inputStream.read(sizeBuffer);
                    if (sizeRead == -1) break;

                    String sizeString = new String(sizeBuffer).trim();
                    if (sizeString.isEmpty()) {
                        Log.e("VideoStreamClient", "Received empty size string");
                        continue;
                    }

                    Log.d("VideoStreamClient", "Received size string: " + sizeString); // 추가된 로그

                    // 카메라 ID와 크기 정보 분리
                    int cameraId = Character.getNumericValue(sizeString.charAt(0));
                    String sizeWithoutId = sizeString.substring(1).trim();
                    if (sizeWithoutId.isEmpty()) {
                        Log.e("VideoStreamClient", "Size without ID is empty");
                        continue;
                    }

                    int frameSize;
                    try {
                        frameSize = Integer.parseInt(sizeWithoutId);
                    } catch (NumberFormatException e) {
                        Log.e("VideoStreamClient", "Received invalid size: " + sizeWithoutId);
                        continue;
                    }

                    byte[] frameBuffer = new byte[frameSize];
                    int bytesRead = 0;
                    while (bytesRead < frameSize) {
                        int result = inputStream.read(frameBuffer, bytesRead, frameSize - bytesRead);
                        if (result == -1) break;
                        bytesRead += result;
                    }

                    Bitmap bitmap = BitmapFactory.decodeByteArray(frameBuffer, 0, frameSize);
                    if (bitmap != null) {
                        publishProgress(new Object[]{bitmap, cameraId});
                    } else {
                        Log.e("VideoStreamClient", "Received null bitmap");
                    }
                }

                socket.close();
            } catch (IOException e) {
                Log.e("VideoStreamClient", "Error: " + e.getMessage());
            }
            return null;
        }

        @Override
        protected void onProgressUpdate(Object[]... params) {
            Bitmap bitmap = (Bitmap) params[0][0];
            int cameraId = (int) params[0][1];

            if (bitmap != null) {
                // 각 ImageView에 맞는 크기로 비트맵 리사이즈
                Bitmap resizedBitmap = resizeBitmap(bitmap, cameraId == 1 ? imageView1.getWidth() : imageView2.getWidth(),
                        cameraId == 1 ? imageView1.getHeight() : imageView2.getHeight());
                if (cameraId == 1) {
                    imageView1.setImageBitmap(resizedBitmap);
                } else if (cameraId == 2) {
                    imageView2.setImageBitmap(resizedBitmap);
                }
            }
        }
    }

    // Bitmap 리사이즈 메소드
    private Bitmap resizeBitmap(Bitmap bitmap, int targetWidth, int targetHeight) {
        if (bitmap == null) return null;

        // 비율 계산
        float aspectRatio = (float) bitmap.getWidth() / (float) bitmap.getHeight();
        int width, height;

        if (targetWidth / (float) targetHeight > aspectRatio) {
            // 세로가 더 좁은 경우
            width = (int) (targetHeight * aspectRatio);
            height = targetHeight;
        } else {
            // 가로가 더 좁은 경우
            width = targetWidth;
            height = (int) (targetWidth / aspectRatio);
        }

        return Bitmap.createScaledBitmap(bitmap, width, height, true);
    }
}
