using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class QualityStats : MonoBehaviour
{
    public GameObject Laser;
    public Text AccuarcyPerSecondValue;
    public Text AccuarcyPerNSecValue;
    public Text AvgDistanceValue;
    public Text AvgDistancePerNSecValue;
    public int SecondsOfLongTimeStats = 10;

    private float AccuarcyCounter = 0;
    private float AccuarcyCounterPerNSec = 0;
    private float AvgDistanceCounter = 0;
    private float AvgDistanceCounterPerNSec = 0;

    private int frameCounter = 0;

    private FrameRateLimiter FrameRate;

    // Start is called before the first frame update
    void Start()
    {
        FrameRate = FrameRateLimiter.Instance;
        if (FrameRate == null)
        {
            Debug.LogError("Frame Rate not specified in FrameRateLimiter script", FrameRate);
        }
    }

    // Update is called once per frame
    void FixedUpdate()
    {
        frameCounter++;

        AvgDistanceCounter += DistanceFromPointToLine(transform.position, Laser.transform.forward, Laser.transform.position) / FrameRate.targetFrameRate;

        if (frameCounter % FrameRate.targetFrameRate == 0) //Every second
        {
            AccuarcyPerSecondValue.text = (AccuarcyCounter * 100 / FrameRate.targetFrameRate) + " %";
            AvgDistanceValue.text = (AvgDistanceCounter).ToString();

            AccuarcyCounterPerNSec += AccuarcyCounter / SecondsOfLongTimeStats;
            AvgDistanceCounterPerNSec += AvgDistanceCounter / SecondsOfLongTimeStats;

            AccuarcyCounter = 0;
            AvgDistanceCounter = 0;
        }
        if (frameCounter % (SecondsOfLongTimeStats * FrameRate.targetFrameRate) == 0) //Every N seconds
        {
            frameCounter = 0;
            AccuarcyPerNSecValue.text = (AccuarcyCounterPerNSec * 100 / FrameRate.targetFrameRate) + " %";
            AvgDistancePerNSecValue.text = (AvgDistanceCounterPerNSec).ToString();

            AccuarcyCounterPerNSec = 0;
            AvgDistanceCounterPerNSec = 0;
        }
    }

    private void OnTriggerStay(Collider other)
    {
        if (other.tag == "Laser")
        {
            if (DistanceFromPointToLine(transform.position, Laser.transform.forward, Laser.transform.position) < 0.35)
            {
                //Debug.Log("Hit");
                AccuarcyCounter++;
            }
        }
    }

    private float DistanceFromPointToLine(Vector3 point, Vector3 lineVector, Vector3 lineOffset)
    {
        return Vector3.Magnitude(Vector3.Cross((lineOffset - point), lineVector)) / Vector3.Magnitude(lineVector);
    }

}
