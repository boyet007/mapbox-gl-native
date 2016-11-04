package com.mapbox.mapboxsdk.maps;

import android.support.annotation.Nullable;
import android.support.annotation.UiThread;

import com.mapbox.mapboxsdk.camera.CameraPosition;
import com.mapbox.mapboxsdk.camera.CameraUpdate;
import com.mapbox.mapboxsdk.geometry.LatLng;

import java.util.concurrent.TimeUnit;

import static com.mapbox.mapboxsdk.maps.MapView.REGION_DID_CHANGE_ANIMATED;

/**
 * Resembles the current Map transformation.
 * <p>
 * Responsible for synchronising {@link CameraPosition} state and notifying {@link com.mapbox.mapboxsdk.maps.MapboxMap.OnCameraChangeListener}.
 * </p>
 */
class Transform {

    private NativeMapView mapView;
    private MapboxMap mapboxMap;

    private CameraPosition cameraPosition;
    private MapboxMap.OnCameraChangeListener onCameraChangeListener;

    Transform(NativeMapView mapView, MapboxMap mapboxMap) {
        this.mapView = mapView;
        this.mapboxMap = mapboxMap;
    }

    //
    // Camera API
    //

    @UiThread
    public final CameraPosition getCameraPosition() {
        return cameraPosition;
    }

    @UiThread
    final void moveCamera(CameraUpdate update, MapboxMap.CancelableCallback callback) {
        cameraPosition = update.getCameraPosition(mapboxMap);
        mapboxMap.getTrackingSettings().resetTrackingModesIfRequired(cameraPosition);
        jumpTo(cameraPosition.bearing, cameraPosition.target, cameraPosition.tilt, cameraPosition.zoom);
        if (callback != null) {
            callback.onFinish();
        }

        // MapChange.REGION_DID_CHANGE_ANIMATED is not called for `jumpTo`
        // invalidate camera position to provide OnCameraChange event.
        mapboxMap.invalidateCameraPosition();
    }

    @UiThread
    final void easeCamera(CameraUpdate update, int durationMs, boolean easingInterpolator, boolean resetTrackingMode, final MapboxMap.CancelableCallback callback) {
        cameraPosition = update.getCameraPosition(mapboxMap);
        if (resetTrackingMode) {
            mapboxMap.getTrackingSettings().resetTrackingModesIfRequired(cameraPosition);
        }

        easeTo(cameraPosition.bearing, cameraPosition.target, getDurationNano(durationMs), cameraPosition.tilt,
                cameraPosition.zoom, easingInterpolator, new MapboxMap.CancelableCallback() {
                    @Override
                    public void onCancel() {
                        if (callback != null) {
                            callback.onCancel();
                        }
                    }

                    @Override
                    public void onFinish() {
                        if (callback != null) {
                            callback.onFinish();
                        }
                    }
                });
    }

    @UiThread
    final void animateCamera(CameraUpdate update, int durationMs, final MapboxMap.CancelableCallback callback) {
        cameraPosition = update.getCameraPosition(mapboxMap);
        mapboxMap.getTrackingSettings().resetTrackingModesIfRequired(cameraPosition);
        flyTo(cameraPosition.bearing, cameraPosition.target, getDurationNano(durationMs), cameraPosition.tilt,
                cameraPosition.zoom, new MapboxMap.CancelableCallback() {
                    @Override
                    public void onCancel() {
                        if (callback != null) {
                            callback.onCancel();
                        }
                    }

                    @Override
                    public void onFinish() {
                        if (callback != null) {
                            callback.onFinish();
                        }
                    }
                });
    }

    @UiThread
    @Nullable
    CameraPosition invalidateCameraPosition() {
        CameraPosition cameraPosition = null;
        if (mapView != null) {
            cameraPosition = new CameraPosition.Builder(mapView.getCameraValues()).build();
            this.cameraPosition = cameraPosition;
            if (onCameraChangeListener != null) {
                onCameraChangeListener.onCameraChange(this.cameraPosition);
            }
        }
        return cameraPosition;
    }

    void jumpTo(double bearing, LatLng center, double pitch, double zoom) {
        mapView.cancelTransitions();
        mapView.jumpTo(bearing, center, pitch, zoom);
    }

    void easeTo(double bearing, LatLng center, long duration, double pitch, double zoom, boolean easingInterpolator, @Nullable final MapboxMap.CancelableCallback cancelableCallback) {
        mapView.cancelTransitions();

        if (cancelableCallback != null) {
            mapView.addOnMapChangedListener(new MapView.OnMapChangedListener() {
                @Override
                public void onMapChanged(@MapView.MapChange int change) {
                    if (change == REGION_DID_CHANGE_ANIMATED) {
                        cancelableCallback.onFinish();
                        mapView.removeOnMapChangedListener(this);
                    }
                }
            });
        }

        mapView.easeTo(bearing, center, duration, pitch, zoom, easingInterpolator);
    }

    void flyTo(double bearing, LatLng center, long duration, double pitch, double zoom, @Nullable final MapboxMap.CancelableCallback cancelableCallback) {
        mapView.cancelTransitions();

        if (cancelableCallback != null) {
            mapView.addOnMapChangedListener(new MapView.OnMapChangedListener() {
                @Override
                public void onMapChanged(@MapView.MapChange int change) {
                    if (change == REGION_DID_CHANGE_ANIMATED) {
                        cancelableCallback.onFinish();
                        mapView.removeOnMapChangedListener(this);
                    }
                }
            });
        }

        mapView.flyTo(bearing, center, duration, pitch, zoom);
    }

    @UiThread
    void resetNorth() {
        mapView.resetNorth();
    }

    void setOnCameraChangeListener(@Nullable MapboxMap.OnCameraChangeListener listener) {
        this.onCameraChangeListener = listener;
    }

    private long getDurationNano(long durationMs) {
        return durationMs > 0 ? TimeUnit.NANOSECONDS.convert(durationMs, TimeUnit.MILLISECONDS) : 0;
    }
}
