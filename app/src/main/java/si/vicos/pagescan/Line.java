package si.vicos.pagescan;

import android.graphics.Canvas;
import android.graphics.Paint;

public class Line implements Overlay {

    private int x1, y1, x2, y2;

    Line(int x1, int y1, int x2, int y2) {
        this.x1 = x1;
        this.y1 = y1;
        this.x2 = x2;
        this.y2 = y2;
    }

    @Override
    public void draw(Canvas canvas, Paint paint) {

        canvas.drawLine(x1, y1, x2, y2, paint);

    }
}
