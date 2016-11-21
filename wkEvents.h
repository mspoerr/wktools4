// Für Eventdeklaration

#ifndef _WKEVENTS_
#define _WKEVENTS_


BEGIN_DECLARE_EVENT_TYPES()
	// Version Check fertig; wird in wktools4 behandelt
	DECLARE_LOCAL_EVENT_TYPE(wkEVT_VCHECK_FERTIG, wxNewEventType())
	
	// Events für wktools4Frame
	DECLARE_LOCAL_EVENT_TYPE(wkEVT_FONTAENDERUNG, wxNewEventType())
	DECLARE_LOCAL_EVENT_TYPE(wkEVT_HIGHLIGHT, wxNewEventType())
	DECLARE_LOCAL_EVENT_TYPE(wkEVT_WKN_FERTIG, wxNewEventType())
	DECLARE_LOCAL_EVENT_TYPE(wkEVT_WKM_FERTIG, wxNewEventType())
	DECLARE_LOCAL_EVENT_TYPE(wkEVT_WKE_FERTIG, wxNewEventType())
	DECLARE_LOCAL_EVENT_TYPE(wkEVT_WKC_FERTIG, wxNewEventType())
	DECLARE_LOCAL_EVENT_TYPE(wkEVT_WKP_FERTIG, wxNewEventType())
	DECLARE_LOCAL_EVENT_TYPE(wkEVT_WKL_FERTIG, wxNewEventType())
	DECLARE_LOCAL_EVENT_TYPE(wkEVT_WKDP_FERTIG, wxNewEventType())
	DECLARE_LOCAL_EVENT_TYPE(wkEVT_WZRD_FERTIG, wxNewEventType())
	DECLARE_LOCAL_EVENT_TYPE(wkEVT_SCHED_FERTIG, wxNewEventType())
	
	// Event an das Log Fenster mit dem entsprechenden Eintrag
	DECLARE_LOCAL_EVENT_TYPE(wkEVT_SCHREIBELOG, wxNewEventType())

	// Event an den Frame, wenn im WKN Propgrid was geändert wurde
	DECLARE_LOCAL_EVENT_TYPE(wkEVT_WKN_PROPGRIDCHANGE, wxNewEventType())

	// Event, wenn im Scheduler PropertyGrid die Profilnamen neu geladen werden sollen
	DECLARE_LOCAL_EVENT_TYPE(wkEVT_SCHED_LADEPROFILE, wxNewEventType())
	// Event, wenn im im Scheduler PropertyGrid der Neu Button gedrückt wird
	DECLARE_LOCAL_EVENT_TYPE(wkEVT_SCHED_NEU, wxNewEventType())
	// Event, wenn Scheduler File neu geladen werden soll
	DECLARE_LOCAL_EVENT_TYPE(wkEVT_SCHED_LADEFILE, wxNewEventType())
	// Event, wenn Reihe markiert wurde
	DECLARE_LOCAL_EVENT_TYPE(wkEVT_SCHED_ROWSELECT, wxNewEventType())
	// Event, wenn Reihe gelöscht werden soll
	DECLARE_LOCAL_EVENT_TYPE(wkEVT_SCHED_ROWDELETE, wxNewEventType())
	
	// Event an WKN, wenn der Ersteller fertig ist
	DECLARE_LOCAL_EVENT_TYPE(wkEVT_ERSTELLER_FERTIG, wxNewEventType())
	// Event an WKE, wenn der Verbinder fertig ist
	DECLARE_LOCAL_EVENT_TYPE(wkEVT_MAPPER_FERTIG, wxNewEventType())
	// Event an WKE, wenn der Parser fertig ist
	DECLARE_LOCAL_EVENT_TYPE(wkEVT_PARSER_FERTIG, wxNewEventType())
	// Event an WKE, wenn der Verbinder fertig ist
	DECLARE_LOCAL_EVENT_TYPE(wkEVT_EINSPIELEN_FERTIG, wxNewEventType())
	// Fehlerlogs von Verbinder an WKE senden
	DECLARE_LOCAL_EVENT_TYPE(wkEVT_EINSPIELEN_FEHLER, wxNewEventType())
	DECLARE_LOCAL_EVENT_TYPE(wkEVT_EINSPIELEN_DEBUGFEHLER, wxNewEventType())
	DECLARE_LOCAL_EVENT_TYPE(wkEVT_EINSPIELEN_INFO, wxNewEventType())
	DECLARE_LOCAL_EVENT_TYPE(wkEVT_EINSPIELEN_SYSTEMFEHLER, wxNewEventType())
END_DECLARE_EVENT_TYPES()


#endif