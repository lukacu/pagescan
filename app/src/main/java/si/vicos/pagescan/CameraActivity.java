package si.vicos.pagescan;

import android.Manifest;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.graphics.Camera;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.graphics.PixelFormat;
import android.graphics.PorterDuff;
import android.media.Image;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.util.Size;
import android.view.SurfaceView;
import android.view.View;
import android.widget.RelativeLayout;
import android.widget.RelativeLayout.LayoutParams;

import java.io.ByteArrayOutputStream;
import java.util.List;
import java.util.Vector;

public class CameraActivity extends Activity implements CameraViewFragment.CameraProcessor {

    private static final String TAG = "CameraActivity";

    private CameraViewFragment mCamera;

    private Handler delayedOperation = new Handler();

    private static Bitmap document;

    public static Bitmap getDocumentBitmap() {

        return document;

    }

    static {

        // Load the native C++ code
        System.loadLibrary("pagescan");

    }

    public CameraActivity() {

        super();

    }

    @Override
    public void onCreate(Bundle savedInstanceState) {

        initializeNative(getAssets());

        super.onCreate(savedInstanceState);

        setContentView(R.layout.cameralayout);

        if (null == savedInstanceState) {
            mCamera = CameraViewFragment.newInstance();
            mCamera.setProcessor(this);

            getFragmentManager().beginTransaction().replace(R.id.container, mCamera)
                    .commit();


        }

    }

    @Override
    protected void onResume() {
        super.onResume();

        View decorView = getWindow().getDecorView();

        // Hide the status bar.
        int uiOptions = View.SYSTEM_UI_FLAG_FULLSCREEN; // | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY;
        decorView.setSystemUiVisibility(uiOptions);

    }

    @Override
    public void processPreview(Image image) {

        /*
        This function processes camera preview image. The first thing to do is to configure transformation
        matrix between the on-screen image and the processed image (the on-screen image is larger, but we
        are only processing a smaller image for obvious performance reasons).
         */

        Quadrilateral q = (Quadrilateral) CameraActivity.processPreviewNative(image);

        /* Example with line detection */
        Vector<Overlay> overlays = new Vector<>();
        if (q != null) {
            overlays.add(q);
            //Log.d(TAG, q.toString());
        } else {

        }

        mCamera.setOverlays(overlays);

    }

    @Override
    public void processCapture(Image image) {

        if (mCamera == null)
            return;

        /*
        This function processes camera capture image. This image is obtained when the user touches the screen
        and is usually much larger in comparison to the preview image.
         */

        if (document != null)
            document.recycle();

        if (CameraActivity.processCaptureNative(image)) {

            Size documentSize = (Size) CameraActivity.retrieveDocumentSize();

            document = Bitmap.createBitmap(documentSize.getWidth(),
                    documentSize.getHeight(), Bitmap.Config.ARGB_8888);

            CameraActivity.retrieveDocumentBitmap(document);

            Intent intent = new Intent(this, DocumentActivity.class);

            startActivity(intent);

        }

    }

    public static native void initializeNative(Object assets);

    public static native Object processPreviewNative(Object image);

    public static native boolean processCaptureNative(Object image);

    public static native Object retrieveDocumentSize();

    public static native boolean retrieveDocumentBitmap(Object image);

}
