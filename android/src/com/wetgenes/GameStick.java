package com.wetgenes;

import android.app.NativeActivity;
import android.util.Log;
import android.content.Intent;
import android.content.Context;
import java.io.File;
import java.io.InputStream;

import java.util.*; 

import com.playjam.gamestick.databaseinterfaceservice.*;
import com.playjam.gamestick.downloadservice.*;


public class GameStick  implements DatabaseInterfaceService.IJavaDatabaseInterfaceResponse
{
	
//	DownloadService m_download_service;

	@Override public void LeaderBoard_GetTop50_Response (LeaderboardData data)
	{
		for (Iterator iterator = data.GetEntries().iterator(); iterator.hasNext();) 
		{
		  LeaderboardData.Entry item = (LeaderboardData.Entry) iterator.next();
		  Log.d("GameStick", Integer.toString( item.AvatarID() ));
		  Log.d("GameStick", item.Name());
		  Log.d("GameStick", Integer.toString( item.Score() ));
		  Log.d("GameStick", Integer.toString( item.Position() ));
		}
	}

	@Override public void LeaderBoard_GetRange_Response (LeaderboardData data)
	{
		for (Iterator iterator = data.GetEntries().iterator(); iterator.hasNext();) 
		{
		  LeaderboardData.Entry item = (LeaderboardData.Entry) iterator.next();
		  Log.d("GameStick", Integer.toString( item.AvatarID() ));
		  Log.d("GameStick", item.Name());
		  Log.d("GameStick", Integer.toString( item.Score() ));
		  Log.d("GameStick", Integer.toString( item.Position() ));
		}
	}

	@Override public void LeaderBoard_GetNearest_Response (LeaderboardData data)
	{
		for (Iterator iterator = data.GetEntries().iterator(); iterator.hasNext();) 
		{
		  LeaderboardData.Entry item = (LeaderboardData.Entry) iterator.next();
		  Log.d("GameStick", Integer.toString( item.AvatarID() ));
		  Log.d("GameStick", item.Name());
		  Log.d("GameStick", Integer.toString( item.Score() ));
		  Log.d("GameStick", Integer.toString( item.Position() ));
		}
   	}

	@Override public void LeaderBoard_SaveScore_Response ()
	{
		Log.d("GameStick", "Save Score Response");
	}

	@Override public void Game_SaveState_Response ()
	{
		Log.d("GameStick", "Saving");
	}

	@Override public void Game_LoadState_Response (InputStream data)
	{
		Log.d("GameStick", "Loading data");
	}

	@Override public void Achievement_GetAllAchievements_Response (Achievements data)
	{
		for (Iterator iterator = data.GetItems().iterator(); iterator.hasNext();) 
		{
			Achievements.Item item = (Achievements.Item) iterator.next();

			Log.d("GameStick", "ID :" + Integer.toString( item.ID() ));
			Log.d("GameStick", "Name : " + item.Name());
			Log.d("GameStick", "Description : " + item.Description());
			Log.d("GameStick", "Type : " + item.Type());
			Log.d("GameStick", "fileName : " + item.FileName());
			Log.d("GameStick", "file URL : " + item.FileURL());
			Log.d("GameStick", "Completed : " + Boolean.toString( item.isCurrentUserOwner() ));
			Log.d("GameStick", "XP Value : " + Integer.toString( item.XPValue() ));
		}
	}

	@Override public void Achievement_SetAchievementComplete_Response ()
	{
		Log.d("GameStick", "Set achievement complete");
	}

	@Override public void Analytics_GameEvent_Response ()
	{
		Log.d("GameStick", "Analytics response");
	}

	@Override public void InAppPurchase_GetPurchasedItems_Response (AppItems data)
	{
		for (Iterator iterator = data.GetItems().iterator(); iterator.hasNext();) 
		{
			AppItems.AppItem item = (AppItems.AppItem) iterator.next();
			Log.d("GameStick", item.toString());
		}
	}

	@Override public void InAppPurchase_GetItemsForPurchase_Response (AppItems data)
	{
		for (Iterator iterator = data.GetItems().iterator(); iterator.hasNext();) 
		{
			AppItems.AppItem item = (AppItems.AppItem) iterator.next();
			Log.d("GameStick", item.toString());
		}
	}

	@Override public void InAppPurchase_PurchaseItem_Response (boolean bought)
	{
		Log.d("GameStick", "Purchase response : " + bought);
   	}

	@Override public void InAppPurchase_GetPurchasedItemURL_Response (String url)
	{
		Log.d("GameStick", "Purchased item url : " + url);
	}

	@Override public void DatabaseRequestFailed (DatabaseInterfaceService.Request id, String message)
	{
		Log.d("GameStick", "Error : " + message);
	}

}
