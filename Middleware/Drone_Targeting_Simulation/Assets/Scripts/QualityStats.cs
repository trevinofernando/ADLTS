using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class QualityStats : MonoBehaviour
{
   
    public TargetingSystem ADLTS;
    public Rigidbody DroneRB;
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
    
    private bool droneWasHit = false;

    private FrameRateLimiter FrameRate;
    private LogManager Log;
    public string LogFileName = "Data.txt";

    private Color RED = new Color(1, 0, 0, 1);
    private Color GREEN = new Color(0.1f, 1, 0, 1);
    private MeshRenderer laserRenderer;
    

    public Material LaserColorHit; //Green
    public Material LaserColorMiss; //Red

    // Start is called before the first frame update
    
    void Start()
    {
        Log = LogManager.Instance;
        FrameRate = FrameRateLimiter.Instance;
        if (FrameRate == null || Log == null)
        {
            Debug.LogError("Frame Rate not specified in FrameRateLimiter script", FrameRate);
            Debug.LogError("Log not specified in FrameRateLimiter script", Log);
        }
        laserRenderer = Laser.GetComponent<MeshRenderer>();
        Log.LogParameters(LogFileName, "ADLTS");
        Log.LogResults(LogFileName, "Entry" + Log.tab + "Acc" + Log.tab + "AvgDistance" + Log.tab + "Velocity\n\n");
    }

    // Update is called once per frame
    void FixedUpdate()
    {
        frameCounter++;

        AvgDistanceCounter += DistanceFromPointToLine(transform.position, Laser.transform.forward, Laser.transform.position) / FrameRate.targetFrameRate;

        laserRenderer.material = droneWasHit ? LaserColorHit : LaserColorMiss;
        droneWasHit = false;

        if (frameCounter % FrameRate.targetFrameRate == 0) //Every second
        {
            float accuarcy = AccuarcyCounter * 100f / (float)FrameRate.targetFrameRate;
            AccuarcyPerSecondValue.text = accuarcy + " %";
            AvgDistanceValue.text = (AvgDistanceCounter).ToString();

            AccuarcyCounterPerNSec += AccuarcyCounter / SecondsOfLongTimeStats;
            AvgDistanceCounterPerNSec += AvgDistanceCounter / SecondsOfLongTimeStats;

            Log.LogResults(Log.logFileName, frameCounter + ":" + Log.tab + accuarcy.ToString("0.000") + Log.tab + AvgDistanceCounter.ToString("0.000") + Log.tab + DroneRB.velocity + "\n");

            AccuarcyPerSecondValue.color = accuarcy < 80f ? RED : GREEN;
            AvgDistanceValue.color = AvgDistanceCounter > 0.45f ? RED : GREEN;

            AccuarcyCounter = 0;
            AvgDistanceCounter = 0;
        }
        if (frameCounter % (SecondsOfLongTimeStats * FrameRate.targetFrameRate) == 0) //Every N seconds
        {
            frameCounter = 0;
            float accuarcy = AccuarcyCounterPerNSec * 100f / (float)FrameRate.targetFrameRate;
            AccuarcyPerNSecValue.text = accuarcy + " %";
            AvgDistancePerNSecValue.text = (AvgDistanceCounterPerNSec).ToString();

            Log.LogResults(Log.logFileName, frameCounter + ":" + Log.tab + accuarcy.ToString("0.000") + Log.tab + AvgDistanceCounterPerNSec.ToString("0.000") + Log.tab + DroneRB.velocity + "\n");

            AccuarcyPerNSecValue.color = accuarcy < 80f ? RED : GREEN;
            AvgDistancePerNSecValue.color = AvgDistanceCounter > 0.45f ? RED : GREEN;

            Debug.Log("Accuarcy: " + accuarcy);
            Debug.Log("Avg Dist: " + AvgDistanceCounterPerNSec);

            AccuarcyCounterPerNSec = 0;
            AvgDistanceCounterPerNSec = 0;
        }
    }

    private void OnTriggerStay(Collider other)
    {
        if (other.tag == "Laser")
        {
            if (DistanceFromPointToLine(transform.position, Laser.transform.forward, Laser.transform.position) < 0.45)
            {
                //Debug.Log("Hit");
                droneWasHit = true;
                AccuarcyCounter++;
            }
        }
    }

    private float DistanceFromPointToLine(Vector3 point, Vector3 lineVector, Vector3 lineOffset)
    {
        return Vector3.Magnitude(Vector3.Cross((lineOffset - point), lineVector)) / Vector3.Magnitude(lineVector);
    }

}
