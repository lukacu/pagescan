package si.vicos.pagescan;

import android.graphics.Canvas;
import android.graphics.Paint;

public class Rectangle implements Overlay {

    private int x, y, width, height;

    public Rectangle(int x, int y, int width, int height) {
        this.x = x;
        this.y = y;
        this.width = width;
        this.height = height;
    }

    @Override
    public void draw(Canvas canvas, Paint paint) {

        canvas.drawRect(x, y, x + width, y+height, paint);

    }
}
