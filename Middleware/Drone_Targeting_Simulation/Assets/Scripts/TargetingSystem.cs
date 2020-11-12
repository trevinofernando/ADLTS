using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class TargetingSystem : MonoBehaviour
{
    public enum Axis{ x = 0, y = 1, z = 2 };
    private Vector3[] AXIS = { new Vector3(1, 0, 0), new Vector3(0, 1, 0), new Vector3(0, 0, 1) };
    public GameObject drone;

    public bool predictVelocity = false;
    public bool frameCompensation = true;
	public bool noiseDampening = false;
    [Range(0f, .25f)] 
    public float MaxNoise = 0f;
    [Range(0f, 1f)] 
    public float chanceToLoseFrame = 0f;
    [Range(0f, 180f)]
    public float velocityMaxDegreeChange = 2f;
    private float cosOfMaxDegreeChange;

    public GameObject MotorForHorizontalMovement; //Turns along the Horizontal axis (MotorY)
    public Axis HorizontalAxis = Axis.x;
    public GameObject MotorForVerticalMovement; //Turns along the vertical axis (MotorX)
    public Axis VerticalAxis = Axis.y;

    public bool angleCamera = true;
	public float MaxLaserRange = 35f; 
	public Transform LaserPosition;
	public Camera cam;
	
    public bool saveLogs = false;

    private Vector2 velocity;
    private Vector2 prevTargetPos = Vector2.zero;
    
    private Vector2 SCREENSIZE;
    
    private bool isFirstRotation = true;
    private bool DroneWasDetectedOnThisFrame;
    private int cyclesSinceLastDetectionOfDrone = 0;
	
	
    private System.Random random = new System.Random();
	private FrameRateLimiter FrameRate;
    private LogManager Log;
    private string LogFileName = "InputOutput.txt";
    //private int frameCounter = 0;
    //private float clock = 0f;

    const double DegToRad = Math.PI / 180f;
    const int clockwise = -1, anticlockwise = 1;

    void Start()
    {
        SCREENSIZE = new Vector2(1f, 1f);
		velocity = new Vector2(0f, 0f);
        isFirstRotation = true;
		
		Log = LogManager.Instance;
		FrameRate = FrameRateLimiter.Instance;
        if (FrameRate == null || Log == null)
        {
            Debug.LogError("Frame Rate not specified in FrameRateLimiter script", FrameRate);
            Debug.LogError("Log not specified in FrameRateLimiter script", Log);
        }
        if (saveLogs)
        {
            Log.LogResults(LogFileName, "targetPositionX" + Log.tab + "targetPositionY" + Log.tab + "AngleX" + Log.tab + "AngleY\n\n");
            Log.LogParameters(LogFileName, "ADLTS");
        }

        //Rotate laser in the direction of the camera for best angle 
        if (LaserPosition != null && MaxLaserRange > 0){
			float laser_camera_distance = cam.transform.position.y - LaserPosition.position.y;
			float angleDiff = (float) (Math.Atan(2f * laser_camera_distance / MaxLaserRange) / DegToRad);
			cam.transform.Rotate(angleDiff * AXIS[(int) Axis.x], Space.Self);
		}
        //Debug.Log("Cos(60): " + Math.Cos(DegToRad * 60f) + " Cos(240): " + Math.Cos(DegToRad * 240f) + " Cos(120): " + Math.Cos(DegToRad * 120f) + " Cos(300): " + Math.Cos(DegToRad * 300f));
        //Debug.Log("Acos(60): " + Math.Acos(Math.Cos(DegToRad * 60f))/ DegToRad + " Acos(240): " + Math.Acos(Math.Cos(DegToRad * 240f))/ DegToRad + " Acos(120): " + Math.Acos(Math.Cos(DegToRad * 120f))/ DegToRad + " Acos(300): " + Math.Acos(Math.Cos(DegToRad * 300f))/ DegToRad);
    }

    // Update is called once per frame
    void FixedUpdate()
    {
        /*Debug.Log("Frame #" + ++frameCounter + "Time since last frame: " + Time.deltaTime);
        frameCounter++;
        clock += Time.deltaTime;
        if (frameCounter % 20 == 0)
        {
            Debug.Log(clock);
            clock = 0;
        }
         */
		

        Vector3 droneCoordinateInCamera = cam.WorldToViewportPoint(drone.transform.position);
        bool onScreen = droneCoordinateInCamera.z > 0 && droneCoordinateInCamera.x > 0 && droneCoordinateInCamera.x < 1 && droneCoordinateInCamera.y > 0 && droneCoordinateInCamera.y < 1;
        //Debug.Log("droneCoordinateInCamera: (" + droneCoordinateInCamera.x + ", "+ droneCoordinateInCamera.y+", "+ droneCoordinateInCamera.z+")");
		
		Vector2 droneCartesiannCoord = new Vector2(droneCoordinateInCamera.x, droneCoordinateInCamera.y);
		Vector2 center = new Vector2(0.5f, 0.5f);
		Vector2 targetPosition = droneCartesiannCoord - center;

        DroneWasDetectedOnThisFrame = true; //defualt flag to true
        if (!onScreen || (float)random.NextDouble() < chanceToLoseFrame)
        {
            isFirstRotation = !onScreen; //If not on screen then set this flag to true
            DroneWasDetectedOnThisFrame = false;
			cyclesSinceLastDetectionOfDrone++;
			
			if(cyclesSinceLastDetectionOfDrone > FrameRate.targetFrameRate){ //lost visual for more than 1 second
                isFirstRotation = true;
                velocity = Vector2.zero; //reset velocity vector to (0,0)
			}
        }
		

		if(DroneWasDetectedOnThisFrame){
			if (MaxNoise > 0f)
			{
				targetPosition.x += GetNoise(MaxNoise);
				targetPosition.y += GetNoise(MaxNoise);
			}

			if (noiseDampening && prevTargetPos != Vector2.zero) //If prevTargetPos = 0 (or minimal) then object is not currently moving so no restrictions on movement direction 
			{
				targetPosition = ReduceNoice(targetPosition, prevTargetPos);
			}
			
			if (predictVelocity && !isFirstRotation)
			{
				if(cyclesSinceLastDetectionOfDrone > 1 && frameCompensation)
                {
                    Vector2 lastPoint = -velocity * cyclesSinceLastDetectionOfDrone; //Travel back to position of last seen drone
                    velocity = (targetPosition - lastPoint) / (cyclesSinceLastDetectionOfDrone + 1); //Assuming no acceleration
                }
                else
                {
				    velocity = (targetPosition / cyclesSinceLastDetectionOfDrone) + velocity; 
                }
				RotateTowards(targetPosition + velocity, cam.fieldOfView, SCREENSIZE);
				prevTargetPos = targetPosition + velocity;
			}else{
				isFirstRotation = false;
				RotateTowards(targetPosition, cam.fieldOfView, SCREENSIZE);
				prevTargetPos = targetPosition;
			}
			
			cyclesSinceLastDetectionOfDrone = 1; //Reset counter. This need to be reset AFTER the velocity is calculated for the current frame.
		}
        else if(predictVelocity)//Drone Was NOT Detected On This Frame
        {
            //Move to future position assuming constant velocity
            RotateTowards(velocity, cam.fieldOfView, SCREENSIZE);
        }
        

    }

    protected void RotateTowards(Vector2 targetPosition, float fieldOfView, Vector2 screenSize)
    {
        float angleX = targetPosition.x * fieldOfView / screenSize.x;
        float angleY = targetPosition.y * fieldOfView / screenSize.y;

        if (saveLogs)
        {
            Log.LogResults(LogFileName, targetPosition.x.ToString("0.000") + ", " + targetPosition.y.ToString("0.000") + Log.tab + angleX.ToString("0.000") + Log.tab + angleY.ToString("0.000") + "\n");
        }

        //Note: that angleX is the angle offset in the horizontal which is controlled by MotorY (that rotates on the Y axis)
        MotorForHorizontalMovement.transform.Rotate(angleX * AXIS[(int) VerticalAxis], Space.Self); //VerticalAxis controls left and right movement
        MotorForVerticalMovement.transform.Rotate(-angleY * AXIS[(int) HorizontalAxis], Space.Self); //HorizontalAxis controls up and down movement
    }
	
	private float GetNoise(float Max_Noise){
			float noise = (float) random.NextDouble() * (Max_Noise);
			if(random.NextDouble() > 0.5f) //50% chance of negative
			{
				noise *= -1;
			}
			//Debug.Log("noice X: " + noice);
			return noise;
	}

	private Vector2 ReduceNoice(Vector2 targetPosition, Vector2 prev_targetPosition){
		cosOfMaxDegreeChange = (float)Math.Cos(DegToRad * (velocityMaxDegreeChange));//field of vision to both sides of velocity vector.
			
		float cosOfAnglePhi = DotProduct2D(Normalized2D(prev_targetPosition), Normalized2D(targetPosition));

		//Debug.Log("Phi: " + Math.Acos(cosOfAnglePhi) / DegToRad + " < " + "MaxDegreeChange: " + Math.Acos(cosOfMaxDegreeChange) / DegToRad);
		if (cosOfAnglePhi > cosOfMaxDegreeChange) //Not inside view cone  CHECK THE SIGN
		{
			//Debug.Log("cosOfAnglePhi: " + cosOfAnglePhi + " > " + "cosOfMaxDegreeChange: " + cosOfMaxDegreeChange);
			Vector2 rotatedVectorCounterClockwise = new Vector2();//vector in the edge of the field of vision (left of prev_targetPosition)
			Vector2 rotatedVectorClockwise = new Vector2();//vector in the edge of the field of vision (right of prev_targetPosition)

			rotatedVectorCounterClockwise.x = (float)(prev_targetPosition.x * Math.Cos(DegToRad * velocityMaxDegreeChange) - prev_targetPosition.y * Math.Sin(DegToRad * velocityMaxDegreeChange));
			rotatedVectorCounterClockwise.y = (float)(prev_targetPosition.x * Math.Sin(DegToRad * velocityMaxDegreeChange) + prev_targetPosition.y * Math.Cos(DegToRad * velocityMaxDegreeChange));
			rotatedVectorClockwise.x = (float)(prev_targetPosition.x * Math.Cos(DegToRad * velocityMaxDegreeChange) + prev_targetPosition.y * Math.Sin(DegToRad * velocityMaxDegreeChange));
			rotatedVectorClockwise.y = (float)(prev_targetPosition.x * -Math.Sin(DegToRad * velocityMaxDegreeChange) + prev_targetPosition.y * Math.Cos(DegToRad * velocityMaxDegreeChange));

			if (cosOfAnglePhi > 0) 
			{
				//Then angle is less than 90 degrees
			}
			else if (cosOfAnglePhi < 0) //Then angle is more than 90 degrees
			{
				//Flip vectors since targetPosition is on the opposite side
				rotatedVectorCounterClockwise *= -1;
				rotatedVectorClockwise *= -1;
				//prev_targetPosition *= -1;                       
			}
			else//(cosOfAnglePhi == 0) Then angle is 90 degrees
			{
				isFirstRotation = true;
				return Vector2.zero; //Unlikely the the drone moved in this direction. Ignore this frame
			}

			//Get projection on the closest rotated vector
			float cosOfCounterClockwiseVector = DotProduct2D(Normalized2D(rotatedVectorCounterClockwise), Normalized2D(targetPosition));
			float cosOfClockwiseVector = DotProduct2D(Normalized2D(rotatedVectorClockwise), Normalized2D(targetPosition));

			if (cosOfCounterClockwiseVector > cosOfClockwiseVector)
			{
				//Then counter clockwise vector is closer 
				return ProjectionOf_U_Onto_V(targetPosition, rotatedVectorCounterClockwise);
			}
			else
			{
				//Then clockwise vector is closer 
				return ProjectionOf_U_Onto_V(targetPosition, rotatedVectorClockwise);
			}
		}
		else
		{
			//targetPosition is already in the direction of posible directions
			return targetPosition;
		}
	}
	
    private Vector2 Normalized2D(Vector2 V)
    {
        /// Returns a vector in the same direction with a magnitude of 1.
        float magnitude = (float) Math.Sqrt(V.x * V.x + V.y * V.y);
        if (magnitude == 0)// Check for 0 before division
            return Vector2.zero;
        return new Vector2(V.x / magnitude, V.y / magnitude);
    } 

    private float DotProduct2D(Vector2 V1, Vector2 V2)
    {
        return V1.x * V2.x + V1.y * V2.y;
    }

    private int Direction(Vector2 v1, Vector2 v2)
    {
        //------------------------ Direction ------------------------------------------
        //
        //  returns positive if v2 is clockwise of this vector,
        //  minus if anticlockwise (Y axis pointing down, X axis to right)
        //------------------------------------------------------------------------
        if (v1.y * v2.x > v1.x * v2.y)
        {
            return anticlockwise;
        }
        else 
        {
            return clockwise;
        }
    }

    private Vector2 ProjectionOf_U_Onto_V(Vector2 U, Vector2 V)
    {
        float scalarMultiple = DotProduct2D(U,V) / DotProduct2D(V,V);
        return scalarMultiple * V;
    }

    private Vector2 Clamp(Vector2 U, Vector2 V)
    {
        return new Vector2((U.x > V.x ? U.x : V.x) , (U.y > V.y ? U.y : V.y));
    }

	
}
